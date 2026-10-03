#!/usr/bin/env perl
# Quick sanity check of the packaging inputs, without running makepkg.
#
# Usage:
#   perl scripts/makepkg.pl             # check PKGBUILD, compile launcher
#   perl scripts/makepkg.pl --no-build  # check PKGBUILD and files only
#
# The launcher is built with gcc into build-pkg/dart.exe (the same command
# the PKGBUILD uses), so run this inside an MSYS2 MINGW64 shell.
use strict;
use warnings;

use File::Basename qw(dirname);
use File::Spec;
use File::Path qw(make_path);

my $root     = File::Spec->catdir(dirname(dirname(__FILE__)));
my $pkgbuild = File::Spec->catfile($root, 'PKGBUILD');
my $outdir   = File::Spec->catdir($root, 'build-pkg');
my $exe      = File::Spec->catfile($outdir, 'dart.exe');
my $src      = File::Spec->catfile($root, 'src', 'launcher.c');

my $no_build = grep { $_ eq '--no-build' } @ARGV;
die "Usage: perl $0 [--no-build]\n" if @ARGV && !(@ARGV == 1 && $no_build);

sub read_file {
    my ($path) = @_;
    open my $fh, '<', $path or die "Unable to open $path: $!\n";
    local $/;
    my $data = <$fh>;
    close $fh;
    return $data;
}

my $pkg = read_file($pkgbuild);
die "PKGBUILD contains CRLF line endings; makepkg cannot source it\n" if $pkg =~ /\r/;

my ($pkgname) = $pkg =~ /^pkgname=(\S+)\s*$/m or die "pkgname not found\n";
my ($pkgver)  = $pkg =~ /^pkgver=(\S+)\s*$/m  or die "pkgver not found\n";
my ($pkgrel)  = $pkg =~ /^pkgrel=(\d+)\s*$/m  or die "pkgrel not found\n";
my ($dart)    = $pkg =~ /^_dart_version=(\S+)\s*$/m or die "_dart_version not found\n";
print "Package: $pkgname $pkgver-$pkgrel (Dart $dart)\n";

($pkg =~ /sha256sums=\(\s*\n\s*'[0-9a-f]{64}'\s*\n?\s*\)/)
    or die "sha256sums must contain exactly one 64-hex entry\n";
print "sha256sums: OK\n";

for my $f ($src, File::Spec->catfile($root, 'LICENSE')) {
    die "Missing file referenced by package(): $f\n" unless -f $f;
}
print "Files: OK\n";

if (!$no_build) {
    make_path($outdir);
    system('gcc', '-O2', '-s', '-municode', '-o', $exe, $src) == 0
        or die "Launcher compilation failed (exit code $?)\n";
    printf "Launcher: OK (%d bytes)\n", -s $exe;
}

print "\nReady for: makepkg -sf\n";
