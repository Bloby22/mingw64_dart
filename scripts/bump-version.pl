#!/usr/bin/env perl
# Bump pkgver/pkgrel in PKGBUILD and sync the version into any CMake project() call.
# Usage:
#   scripts/bump.pl 0.2.0            # set version + reset pkgrel to 1
#   scripts/bump.pl --major          # 0.1.0 -> 1.0.0
#   scripts/bump.pl --minor          # 0.1.0 -> 0.2.0
#   scripts/bump.pl --patch          # 0.1.0 -> 0.1.1
#   scripts/bump.pl --bump-pkgrel    # keep version, pkgrel + 1
use strict;
use warnings;

use File::Basename qw(dirname);
use File::Spec;

my $root = File::Spec->catdir(dirname(dirname(__FILE__)));
my $pkgbuild = File::Spec->catfile($root, 'PKGBUILD');
my $cmakelists = File::Spec->catfile($root, 'CMakeLists.txt');

sub read_file {
    my ($path) = @_;
    open my $fh, '<', $path or die "Unable to open $path: $!\n";
    local $/;
    my $data = <$fh>;
    close $fh;
    return $data;
}

sub write_file {
    my ($path, $data) = @_;
    open my $fh, '>', $path or die "Unable to write $path: $!\n";
    print {$fh} $data;
    close $fh;
    return;
}

# Replace exactly one `field = ...` line with `field = value`.
sub set_field {
    my ($data_ref, $field, $value, $path) = @_;
    my $n = ($$data_ref =~ s/^(\s*\Q$field\E\s*=\s*).*\n/$1$value\n/mg);
    die "Expected exactly one $field in $path, found $n\n" if $n != 1;
    return;
}

sub read_version {
    my ($data, $path) = @_;
    ($data =~ /^\s*pkgver\s*=\s*(\d+(?:\.\d+)*)\s*$/m)
        or die "Could not find pkgver in $path\n";
    return $1;
}

sub read_pkgrel {
    my ($data, $path) = @_;
    return ($data =~ /^\s*pkgrel\s*=\s*(\d+)\s*$/m) ? $1 : 1;
}

sub bump {
    my ($ver, $which) = @_;
    my @p = split /\./, $ver;
    while (@p < 3) { push @p, 0; }
    if    ($which eq 'major') { @p = ($p[0] + 1, 0, 0); }
    elsif ($which eq 'minor') { @p = ($p[0], $p[1] + 1, 0); }
    else                      { @p = ($p[0], $p[1], $p[2] + 1); }
    return join '.', @p;
}

my $mode = shift @ARGV // '';
if (!grep { $mode eq $_ } qw(--major --minor --patch --bump-pkgrel)) {
    $mode = shift @ARGV if $mode eq '' && @ARGV;
    die "Usage: $0 [--major|--minor|--patch|--bump-pkgrel] <new-version>\n"
        . "  or:     $0 0.2.0\n" unless defined($mode) && $mode =~ /^\d+(?:\.\d+)*$/;
}

my $data = read_file($pkgbuild);
my $old = read_version($data, $pkgbuild);

if ($mode eq '--bump-pkgrel') {
    my $rel = read_pkgrel($data, $pkgbuild) + 1;
    set_field(\$data, 'pkgrel', $rel, $pkgbuild);
    write_file($pkgbuild, $data);
    print "$pkgbuild: pkgrel -> $rel (pkgver $old unchanged)\n";
    exit 0;
}

my $new;
if ($mode =~ /^\d+(?:\.\d+)*$/) {
    $new = $mode;
} else {
    my $which = $mode;
    $which =~ s/^--//;
    $new = bump($old, $which);
}

$new =~ /^\d+(?:\.\d+)*$/ or die "Invalid version: $new\n";

set_field(\$data, 'pkgver', $new, $pkgbuild);
set_field(\$data, 'pkgrel', 1, $pkgbuild);
write_file($pkgbuild, $data);
print "$pkgbuild: $old -> $new (pkgrel reset to 1)\n";

# Sync CMakeLists.txt project(... VERSION x.y.z) when present.
if (-f $cmakelists) {
    my $cmake = read_file($cmakelists);
    my $n = ($cmake =~ s/^(\s*project\s*\(\s*\w+\s+VERSION\s+)\d+(?:\.\d+)*/$1$new/m);
    if ($n) {
        die "Expected exactly one project(VERSION) in $cmakelists, found $n\n" if $n != 1;
        write_file($cmakelists, $cmake);
        print "$cmakelists: project VERSION -> $new\n";
    }
}