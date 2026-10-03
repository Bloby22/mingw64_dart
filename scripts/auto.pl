#!/usr/bin/env perl
# Check for new stable Dart / Flutter releases and update PKGBUILD.
#
# Updates _dart_version, _flutter_version and sha256sums, bumps pkgver (patch)
# and resets pkgrel to 1. Hashes come from the official Google Storage
# metadata; makepkg verifies them again when it downloads the archives.
#
# Usage:
#   scripts/auto.pl                 # update PKGBUILD if newer stable versions exist
#   scripts/auto.pl --check         # only report; exit 10 if an update is available
#   scripts/auto.pl --dart 3.13.5   # force a Dart version (hash still looked up)
#   scripts/auto.pl --flutter 3.47.6
#   scripts/auto.pl --no-bump       # do not touch pkgver/pkgrel
#
# Requires: perl (core modules only) and curl.
use strict;
use warnings;

use File::Basename qw(dirname);
use File::Spec;
use JSON::PP qw(decode_json);

my $DART_BASE = $ENV{DART_BASE}
    // 'https://storage.googleapis.com/dart-archive/channels/stable/release';
my $FLUTTER_META = $ENV{FLUTTER_META}
    // 'https://storage.googleapis.com/flutter_infra_release/releases/releases_windows.json';
my $DART_ARCHIVE = 'dartsdk-windows-x64-release.zip';

my ($check, $no_bump, $force_dart, $force_flutter) = (0, 0, undef, undef);
while (my $a = shift @ARGV) {
    if    ($a eq '--check')   { $check = 1 }
    elsif ($a eq '--no-bump') { $no_bump = 1 }
    elsif ($a eq '--dart')    { $force_dart = shift @ARGV // die "--dart needs a version\n" }
    elsif ($a eq '--flutter') { $force_flutter = shift @ARGV // die "--flutter needs a version\n" }
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

($pb =~ /^_dart_version=(\S+)\s*$/m)    or die "_dart_version not found in PKGBUILD\n";
my $cur_dart = $1;
($pb =~ /^_flutter_version=(\S+)\s*$/m) or die "_flutter_version not found in PKGBUILD\n";
my $cur_flutter = $1;

# --- Dart ---
my $new_dart = $force_dart;
if (!defined $new_dart) {
    my $j = decode_json(fetch("$DART_BASE/latest/VERSION"));
    $new_dart = $j->{version} // die "No version in Dart VERSION file\n";
}
my $dart_line = fetch("$DART_BASE/$new_dart/sdk/$DART_ARCHIVE.sha256sum");
($dart_line =~ /^([0-9a-f]{64})\b/i) or die "Cannot parse Dart sha256 for $new_dart\n";
my $dart_sha = lc $1;

# --- Flutter ---
my $meta = decode_json(fetch($FLUTTER_META));
my $new_flutter = $force_flutter;
if (!defined $new_flutter) {
    my $hash = $meta->{current_release}{stable} // die "No stable release in Flutter metadata\n";
    my ($rel) = grep { ($_->{hash} // '') eq $hash && ($_->{channel} // '') eq 'stable' }
                     @{ $meta->{releases} };
    die "Stable release $hash not found in Flutter metadata\n" unless $rel;
    $new_flutter = $rel->{version};
}
my ($frel) = grep { ($_->{channel} // '') eq 'stable' && ($_->{version} // '') eq $new_flutter }
                  @{ $meta->{releases} };
die "Flutter $new_flutter not found in stable channel\n" unless $frel;
my $flutter_sha = lc($frel->{sha256} // '');
($flutter_sha =~ /^[0-9a-f]{64}$/) or die "No valid sha256 for Flutter $new_flutter\n";

for my $v ($new_dart, $new_flutter) {
    $v =~ /^\d+(?:\.\d+)*$/ or die "Unexpected version string: $v\n";
}

my $dart_changed    = $new_dart ne $cur_dart;
my $flutter_changed = $new_flutter ne $cur_flutter;

printf "Dart:    %s -> %s%s\n", $cur_dart, $new_dart, $dart_changed ? '' : ' (unchanged)';
printf "Flutter: %s -> %s%s\n", $cur_flutter, $new_flutter, $flutter_changed ? '' : ' (unchanged)';

if (vcmp($new_dart, $cur_dart) < 0 || vcmp($new_flutter, $cur_flutter) < 0) {
    warn "WARNING: a selected version is older than the one in PKGBUILD.\n";
}

# Current hashes (also lets us notice a changed hash for the same version)
($pb =~ /sha256sums=\(\s*\n\s*'([0-9a-f]{64})'\s*\n\s*'([0-9a-f]{64})'\s*\n?\s*\)/)
    or die "Could not parse the two-entry sha256sums array in PKGBUILD\n";
my ($cur_dart_sha, $cur_flutter_sha) = ($1, $2);
my $hash_changed = $cur_dart_sha ne $dart_sha || $cur_flutter_sha ne $flutter_sha;

if (!$dart_changed && !$flutter_changed && !$hash_changed) {
    print "PKGBUILD is up to date.\n";
    exit 0;
}

if ($check) {
    print "Update available.\n";
    exit 10;
}

$pb =~ s/^(_dart_version=).*$/$1$new_dart/m;
$pb =~ s/^(_flutter_version=).*$/$1$new_flutter/m;
$pb =~ s/(sha256sums=\(\s*\n\s*')[0-9a-f]{64}('\s*\n\s*')[0-9a-f]{64}(')/$1$dart_sha$2$flutter_sha$3/
    or die "Failed to rewrite sha256sums\n";

if (!$no_bump && ($dart_changed || $flutter_changed)) {
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
