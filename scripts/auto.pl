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
    elsif ($a eq '--dart')    { $force_dart = shift @ARGV // die "--dart needs a version\n" }
    else { die "Unknown argument: $a\n" }
}

my $root = File::Spec->catdir(dirname(dirname(__FILE__)));
my $pkgbuild = File::Spec->catfile($root, 'PKGBUILD');

sub read_file {
    my ($p) = @_;
    open my $fh, '<', $p or die "Unable to open $p: $!\n";
    local $/;
    my $d = <$fh>;
    close $fh;
    return $d;
}

sub write_file {
    my ($p, $d) = @_;
    open my $fh, '>', $p or die "Unable to write $p: $!\n";
    binmode $fh;
    print {$fh} $d;
    close $fh;
}

sub fetch {
    my ($url) = @_;
    my $out = `curl -fsSL --retry 3 "$url"`;
    die "Download failed: $url\n" if $? != 0 || !defined $out || $out eq '';
    return $out;
}

sub vcmp {    # compare dotted versions numerically
    my ($x, $y) = @_;
    my @a = split /\./, $x;
    my @b = split /\./, $y;
    while (@a || @b) {
        my $r = (shift(@a) // 0) <=> (shift(@b) // 0);
        return $r if $r;
    }
    return 0;
}

my $pb = read_file($pkgbuild);

($pb =~ /^_dart_version=(\S+)\s*$/m) or die "_dart_version not found in PKGBUILD\n";
my $cur_dart = $1;

# --- Dart ---
my $new_dart = $force_dart;
if (!defined $new_dart) {
    my $j = decode_json(fetch("$DART_BASE/latest/VERSION"));
    $new_dart = $j->{version} // die "No version in Dart VERSION file\n";
}
$new_dart =~ /^\d+(?:\.\d+)*$/ or die "Unexpected version string: $new_dart\n";

my $dart_line = fetch("$DART_BASE/$new_dart/sdk/$DART_ARCHIVE.sha256sum");
($dart_line =~ /^([0-9a-f]{64})\b/i) or die "Cannot parse Dart sha256 for $new_dart\n";
my $dart_sha = lc $1;

my $dart_changed = $new_dart ne $cur_dart;
printf "Dart: %s -> %s%s\n", $cur_dart, $new_dart, $dart_changed ? '' : ' (unchanged)';

if (vcmp($new_dart, $cur_dart) < 0) {
    warn "WARNING: selected version is older than the one in PKGBUILD.\n";
}

# Current hash (also lets us notice a changed hash for the same version).
# Tolerant of any formatting: one line or many, ' or " quotes, comments, CRLF.
($pb =~ /^sha256sums=\(([^)]*)\)/m)
    or die "sha256sums=(...) array not found in PKGBUILD\n";
my $sums_body = $1;
my @cur_hashes = $sums_body =~ /\b([0-9a-fA-F]{64})\b/g;
die "Expected exactly one sha256 in PKGBUILD sha256sums, found " . scalar(@cur_hashes) . "\n"
    if @cur_hashes != 1;
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

$pb =~ s/^(_dart_version=).*$/$1$new_dart/m;
$pb =~ s/^(sha256sums=\([^)]*?)\b[0-9a-fA-F]{64}\b/$1$dart_sha/m
    or die "Failed to rewrite sha256sums\n";

if (!$no_bump && $dart_changed) {
    ($pb =~ /^pkgver=(\d+(?:\.\d+)*)\s*$/m) or die "pkgver not found\n";
    my @p = split /\./, $1;
    push @p, 0 while @p < 3;
    $p[2]++;
    my $new_ver = join '.', @p;
    $pb =~ s/^pkgver=.*$/pkgver=$new_ver/m;
    $pb =~ s/^pkgrel=.*$/pkgrel=1/m;
    print "pkgver -> $new_ver (pkgrel reset to 1)\n";
}

write_file($pkgbuild, $pb);
print "PKGBUILD updated.\n";
