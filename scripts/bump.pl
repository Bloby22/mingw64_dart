#!/usr/bin/env perl
# Packaging sanity check: rebuild the PKGBUILD g++ command and verify the
# resulting package layout, without needing makepkg or a pacman environment.
#
# Usage:
#   perl scripts/makepkg-check.pl           # build into build-pkg/ and verify
#   perl scripts/makepkg-check.pl --no-build  # verify existing build-pkg/ only
use strict;
use warnings;

use File::Basename qw(dirname);
use File::Spec;
use File::Path qw(make_path);

my $root = File::Spec->catdir(dirname(dirname(__FILE__)));
my $pkgbuild = File::Spec->catfile($root, 'PKGBUILD');
my $outdir = File::Spec->catdir($root, 'build-pkg');
my $exe = File::Spec->catfile($outdir, 'release_client.exe');

my $no_build = grep { $_ eq '--no-build' } @ARGV;
if (@ARGV && !$no_build) {
    die "Usage: perl $0 [--no-build]\n";
}

sub read_file {
    my ($path) = @_;
    open my $fh, '<', $path or die "Unable to open $path: $!\n";
    local $/;
    my $data = <$fh>;
    close $fh;
    return $data;
}

my $pkg = read_file($pkgbuild);
my ($pkgname) = $pkg =~ /^\s*pkgname\s*=\s*(\S+)\s*$/m
    or die "Could not find pkgname in $pkgbuild\n";
my ($pkgver) = $pkg =~ /^\s*pkgver\s*=\s*(\S+)\s*$/m
    or die "Could not find pkgver in $pkgbuild\n";
my ($pkgrel) = $pkg =~ /^\s*pkgrel\s*=\s*(\d+)\s*$/m
    or die "Could not find pkgrel in $pkgbuild\n";

print "Package: $pkgname $pkgver-$pkgrel\n";

# Sources listed in PKGBUILD build() must exist and compile.
my @sources = $pkg =~ /^\s+"?\$startdir\/(src\/[\w\/.]+\.cpp)"?\s*\\?$/mg;
die "No source files found in $pkgbuild\n" unless @sources;
for my $src (@sources) {
    my $path = File::Spec->catfile($root, split m{/}, $src);
    die "Missing source: $src\n" unless -f $path;
}
printf "Sources (%d): OK\n", scalar @sources;

if (!$no_build) {
    make_path($outdir);
    # Mirror the g++ invocation from PKGBUILD build():
    # split into tokens, drop line-continuation backslashes, remove shell
    # quotes and expand $startdir anywhere inside a token.
    my ($gxx_line) = $pkg =~ /g\+\+(.*?)\n\s*-lcurl/s
        or die "Could not find the g++ command in $pkgbuild\n";
    my @flags;
    for my $tok (split ' ', $gxx_line) {
        next if $tok eq '\\';
        $tok =~ s/"//g;
        $tok =~ s/\$startdir/$root/g;
        push @flags, $tok;
    }
    print "Compile: g++ @flags -lcurl\n";
    chdir $root or die "Cannot enter $root: $!\n";
    system('g++', @flags, '-lcurl') == 0
        or die "Compilation failed (exit code $?)\n";
    print "Build: OK\n";
}

die "Missing $exe - run without --no-build\n" unless -f $exe;

# Verify the artifacts package() would install.
my $license = File::Spec->catfile($root, 'LICENSE');
die "Missing LICENSE (installed under mingw64/share/licenses/...)\n" unless -f $license;

my $size = -s $exe;
printf "Result: OK\n  %s (%d bytes)\n  LICENSE (present)\n",
       $exe, $size;

# Smoke test: binary must at least respond to --help.
my $help_out = `$exe --help 2>&1`;
if ($? == 0 && $help_out =~ /Usage/) {
    print "Smoke test (--help): OK\n";
} else {
    print "Smoke test (--help): WARNING - output or exit code is unexpected\n";
}

print "\nPackage ready for packaging: $pkgname $pkgver-$pkgrel\n";
