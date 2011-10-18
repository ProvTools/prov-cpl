#!/usr/bin/perl

$col_gray =         "\033[37m";
$col_purple =       "\033[35m";
$col_green =        "\033[32m";
$col_cyan =         "\033[36m";
$col_brown =        "\033[33m";
$col_red =          "\033[31m";
$col_blue =         "\033[34m";

$col_brighten =     "\033[01m";
$col_underline =    "\033[04m";
$col_normal =       "\033[0;0m";

$col_this_is =      $col_cyan . $col_brighten;
$col_no_file =      $col_red;
$col_error =        $col_red . $col_brighten;
$col_warning =      $col_brown . $col_brighten;
$col_underfull =    $col_purple . $col_brighten;

$join_lines = 1;
$remove_empty_lines = 1;
$remove_help_suggestions = 1;
$remove_packages = 1;
$remove_see_transcript = 1;

$in_transcript = undef;
$thisline = "";
$prefix = "";

while (<>)
{
	$orgline = $_;
	$thisline = $prefix . $orgline;

	# Remove multiple spaces
	$thisline =~ s/  \+/ /g;

	# Remove end of line symbols
	$thisline =~ s/[\n\r]//g;


	#
	# Join split lines
	#

	if ($join_lines) {
		if ((length $orgline) == 80) {
			$prefix = $thisline;
			next;
		} else {
			$prefix = "";
		}
	}


	#
	# Remove
	#

	# Remove empty lines
	if ($remove_empty_lines && ($thisline =~ /^[ \t]*$/)) {
		next;
	}

	# For additional information...
	if ($remove_help_suggestions) {
		if ($thisline =~ /^(For\ additional\ information)/) {
			next;
		}
	}

	# Remove package information
	if ($remove_packages) {

		# Package: `...'
		if ($thisline =~ /^(Package:\ \`)/) {
			next;
		}

		# (/usr/share/texmf-texlive/tex/latex/amsmath/amsmath.sty...
		$b = $thisline =~ s/^[\( \)]*    \/.*\.sty    [\( \)]*//x;
		$b = $thisline =~ s/^[\( \)]*    \/.*\.cfg    [\( \)]*//x || $b;
		$b = $thisline =~ s/^[\( \)]*    \/.*\.def    [\( \)]*//x || $b;
		$b = $thisline =~ s/^[\( \)]*    \/.*\.clo    [\( \)]*//x || $b;
		$b = $thisline =~ s/^[\( \)]*    \/.*\.cls    [\( \)]*//x || $b;
		$b = $thisline =~ s/^[\( \)]*  \.\/.*\.aux    [\( \)]*//x || $b;
		$b = $thisline =~ s/^[\( \)]*    \/.*\.fd     [\( \)]*//x || $b;
		$b = $thisline =~ s/^[ \t]*[\(\)]*//x || $b;
		if ($b && ($thisline =~ /^[ \t]*$/)) { next; }
	}

	# Remove transcript information
	if ($remove_see_transcript) {

		# Output written on...
		if ($thisline =~ /^(Output\ written)/) {
			$in_transcript = undef;
		}

		# see the transcript file for additional information...
		if ($thisline =~ /^(see\ the\ transcript)/) {
			$in_transcript = 1;
		}

		# ...everything in between...
		if ($in_transcript) {
			next;
		}
	}


	#
	# Color: pdflatex
	#

	# LaTeX Warning: ...
	$thisline =~ s/^(LaTeX\ Warning)/$col_warning$1/x;

	# Warning--...
	$thisline =~ s/^(Warning--)/$col_warning$1/x;

	# Underfull ...
	$thisline =~ s/^(Underfull)/$col_underfull$1/x;

	# No file ...
	$thisline =~ s/^(No\ file)/$col_no_file$1/x;

	# ./report.tex:78: Undefined...
	$thisline =~ s/^(.*\.tex:)/$col_error$1/x;


	#
	# Color: bibtex
	#

	# There was 1 error message
	$thisline =~ s/^(There\ was\ [0-9]+\ error)/$col_error$1/x;

	# I found no...
	$thisline =~ s/^(I\ found\ no)/$col_error$1/x;

	# I was...
	$thisline =~ s/^(I\ was)/$col_error$1/x;


	#
	# Extra line breaks
	#

	# This is ...
	if ($thisline =~ s/^(This\ is)/$col_this_is$1/x) {
		print "\n";
	}


	#
	# Print
	#

	print $thisline . "\n";
	print $col_normal;
	$thisline = "";
}


# Finalize

if ((length $thisline) > 0) {
	print $thisline . "\n";
	print $col_normal;
}
