
#!/usr/bin/env perl
use strict;
use warnings;

use File::Basename qw(dirname);
use File::Spec;
use JSON::PP qw(decode_json);

my $DART_BASE = $ENV{DART_BASE}
    // 'https://storage.googleapis.com/dart-archive/channels/stable/release';
my $DART_ARCHIVE = 'dartsdk-windows-x64-release.zip';

my ($check, $no_bump, $force_dart) = (0, 0, undef);

while (my $a = shift @ARGV) {
    if    ($a eq '--check')   { $check = 1 }
    elsif ($a eq '--no-bump') { $no_bump = 1 }
    elsif ($a eq '--dart') {
        $force_dart = shift @ARGV // die "--dart needs a version\n";
    }
    else {
        die "Unknown argument: $a\n";
    }
}

my $root = File::Spec->rel2abs(
    File::Spec->catdir(dirname(__FILE__), '..')
);
my $pkgbuild = File::Spec->catfile($root, 'PKGBUILD');

sub read_file {
    my ($path) = @_;

    open my $fh, '<', $path
        or die "Unable to open $path: $!\n";

    local $/;
    my $data = <$fh>;
    close $fh or die "Unable to close $path: $!\n";

    return $data;
}

sub write_file {
    my ($path, $data) = @_;

    open my $fh, '>', $path
        or die "Unable to write $path: $!\n";

    binmode $fh;
    print {$fh} $data
        or die "Failed to write $path: $!\n";
    close $fh
        or die "Unable to close $path: $!\n";
}

sub fetch {
    my ($url) = @_;

    my $out = `curl -fsSL --retry 3 "$url"`;
    die "Download failed: $url\n"
        if $? != 0 || !defined($out) || $out eq '';

    return $out;
}

sub vcmp {
    my ($x, $y) = @_;

    my @a = split /\./, $x;
    my @b = split /\./, $y;

    while (@a || @b) {
        my $left  = shift(@a) // 0;
        my $right = shift(@b) // 0;

        my $result = $left <=> $right;
        return $result if $result;
    }

    return 0;
}

my $pb = read_file($pkgbuild);

$pb =~ /^_dart_version=(\d+(?:\.\d+)*)\s*$/m
    or die "_dart_version not found in PKGBUILD\n";

my $cur_dart = $1;

# Get the requested or latest stable Dart version.
my $new_dart = $force_dart;

if (!defined $new_dart) {
    my $json = decode_json(
        fetch("$DART_BASE/latest/VERSION")
    );

    $new_dart = $json->{version}
        // die "No version in Dart VERSION file\n";
}

$new_dart =~ /^\d+(?:\.\d+)*$/
    or die "Unexpected version string: $new_dart\n";

# Get the SHA-256 checksum for the Windows SDK archive.
my $dart_line = fetch(
    "$DART_BASE/$new_dart/sdk/$DART_ARCHIVE.sha256sum"
);

$dart_line =~ /^([0-9a-f]{64})\b/i
    or die "Cannot parse Dart SHA-256 for $new_dart\n";

my $dart_sha = lc $1;

my $dart_changed = $new_dart ne $cur_dart;

printf "Dart: %s -> %s%s\n",
    $cur_dart,
    $new_dart,
    $dart_changed ? '' : ' (unchanged)';

if (vcmp($new_dart, $cur_dart) < 0) {
    warn "WARNING: selected version is older than the current version.\n";
}

# Find the checksum array and its existing SHA-256.
$pb =~ /^sha256sums=\(([^)]*)\)/m
    or die "sha256sums=(...) array not found in PKGBUILD\n";

my $sums_body = $1;
my @cur_hashes = $sums_body =~ /\b([0-9a-fA-F]{64})\b/g;

@cur_hashes == 1
    or die "Expected exactly one SHA-256 in PKGBUILD sha256sums, found "
        . scalar(@cur_hashes) . "\n";

my $cur_dart_sha = lc $cur_hashes[0];
my $hash_changed = $cur_dart_sha ne $dart_sha;

if (!$dart_changed && !$hash_changed) {
    print "PKGBUILD is up to date.\n";
    exit 0;
}

if ($check) {
    print "Update available.\n";
    exit 10;
}

# Update the Dart SDK version.
$pb =~ s/^_dart_version=.*$/_dart_version=$new_dart/m
    or die "Failed to update _dart_version\n";

# Update only the first SHA-256 entry in sha256sums.
$pb =~ s/^(
    sha256sums=\(
    [^)]*?
    )
    \b[0-9a-fA-F]{64}\b
/$1$dart_sha/mx
    or die "Failed to update sha256sums\n";

# Set pkgver to the actual Dart version.
# Supports both a numeric pkgver and the original Bash expression:
# pkgver=${_dart_version//-/_}
if (!$no_bump && $dart_changed) {
    $pb =~ /^pkgver=.*$/m
        or die "pkgver assignment not found in PKGBUILD\n";

    $pb =~ s/^pkgver=.*$/pkgver=$new_dart/m
        or die "Failed to update pkgver\n";

    $pb =~ /^pkgrel=.*$/m
        or die "pkgrel assignment not found in PKGBUILD\n";

    $pb =~ s/^pkgrel=.*$/pkgrel=1/m
        or die "Failed to reset pkgrel\n";

    print "pkgver -> $new_dart (pkgrel reset to 1)\n";
}

write_file($pkgbuild, $pb);

print "Dart version: $new_dart\n";
print "SHA-256: $dart_sha\n";
print "PKGBUILD updated successfully.\n";
