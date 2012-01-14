#!/usr/bin/perl
################################################################################
################################################################################
### SwapInserter
### Author: Brandon Henderson
### Created on: 22Dec08
### Last Edited on: 23Jan09
###
### See $HELP_MESSAGE for program description
###
	my $POST = "-swp";
	my $SWAPS_OFF = "\@DISABLE_SWAPS";
	my $SWAPS_ON = "\@ENABLE_SWAPS";
	my $F_MAX = 10;
	my $F_DEF = 1;
	my $T_MAX = 8;
	my $T_DEF = 3;
	my $R_MAX = 100;
	my $R_DEF = 80;
	my $HELP_MESSAGE = "\n".
"   Usage: perl $0 <inputFile> <options>"                                       ."\n".
                                                                                 "\n".
"   The outputFile is the inputFile's name with a postfix '$POST'. It will be"  ."\n".
"   a duplicate of the inputFile, except for SWAPs appended at the right"       ."\n".
"   margin on appropriate lines according to the swap frequency. A different"   ."\n".
"   outputFile may be specified if desired. Change tab width and right margin"  ."\n".
"   to be those of your development environment for SWAP prettyprint."          ."\n".
                                                                                 "\n".
"   To control SWAP placement, manually insert into the inputFile these"        ."\n".
"   directives anywhere (commented out):"                                       ."\n".
"      $SWAPS_OFF    Lines after this CANNOT have SWAPs appended."              ."\n".
"      $SWAPS_ON     Lines after this CAN have SWAPs appended."                 ."\n".
                                                                                 "\n".
"   Style choices for best SWAP placement (optional):"                          ."\n".
"      1. Always use curly braces on every keyword construct."                  ."\n".
"      2. Don't put curly braces on the same line."                             ."\n".
"      3. Don't put code on the same line as a curly brace."                    ."\n".
                                                                                 "\n".
"   Options:"                                                                   ."\n".
"      -o <outputFile>       specify different outputFile."                     ."\n".
"      -f <swap frequency>   every 1 to $F_MAX line(s). Default is $F_DEF."     ."\n".
"      -t <tab width>        1 to $T_MAX. Default is $T_DEF."                   ."\n".
"      -r <right margin>     1 to $R_MAX characters. Default is $R_DEF."        ."\n".
"      -v                    verbose display"                                   ."\n".
"      -s                    skip variable declarations (for MS Visual Studio)."."\n".
"                               You can manually add your own variable types to"."\n".
"                               the array \@VARIABLE_TYPES in this program."    ."\n".
"      -d                    debug data"                                        ."\n"
                                                                                ."\n";
################################################################################
################################################################################
no warnings 'deprecated';

my $swapFrequency = $F_DEF;
my $tabWidth = $T_DEF;
my $rightMargin = $R_DEF;
my $verbose = 0;
my $skipVarDecls = 0;
my @VARIABLE_TYPES = (
		# BASICS
			"int","char","short","long","float","double","void","struct","const",
			"(un)?signed","extern","static",
		# EXTENDED
			"bool(ean)?","byte","Semaphore","time_t","clock_t","TCB","Message",
			"TID","Account","CAR","JPARK","PC","DiskSize","FDEntry","BSStruct",
			"FATTime","FATDate","DirEntry","FMSERROR","Command","FILE",
		# ADD YOUR OWN VARIABLE TYPES HERE !!! (NO POINTERS)
			"YOUR TYPE HERE"
	);
my $debug = 0;

my $CURLY_MARK = "\x0";
my $SWAP_MARK = "\x1";
my $SWAP_CODE = ";SWAP;";
my $debugBar;

main( @ARGV );
exit;




################################################################################
sub main(){
	# GRAB INPUT FILE NAME
	my $inputFile = shift or die $HELP_MESSAGE;

	# OPEN INPUT FILE, CHECK EXISTENCE
	open READFILE, $inputFile
		or die "\n   File `$inputFile' does not exist.\n$HELP_MESSAGE";

	# OUTPUT FILE IS "infilename$POST.ext"
	my $outputFile = "$inputFile$POST";
	if( $inputFile =~ m/(.*)\./ ){ $outputFile = "$1$POST.$'"; }

	# PARSE COMMANDLINE ARGS AFTER INPUTFILE
	while( 1 ){  # while there is an arg
		$_ = shift; # dequeue arg
		if( $_ eq "" ){ last; } # if arg blank, end while

		if( m/^-v/i ){ $verbose = 1; # CHECK FOR VERBOSE OPTION
		}elsif( m/^-s/i ){ $skipVarDecls = 1; # CHECK FOR 'SKIP VAR DECLS' OPTION
		}elsif( m/^-d/i ){ $debug = 1; # CHECK FOR DEBUG OPTION
		}elsif( m/^-o/i ){# CHECK FOR OUTPUT FILE OPTION
			if( $' eq "" ){ $outputFile = shift; } # value is next arg
			else{ $outputFile = $'; } # value is rest of this arg
		}elsif( m/^-f/i ){# CHECK FOR FREQUENCY OPTION
			if( $' eq "" ){ $_ = shift; } # value is next arg
			else{ $_ = $'; } # value is rest of this arg
			# check value is number and within range
			if( !( m/\d+/ ) ){ next; }
			if( $_ < 1 ){ $_ = 1; }
			elsif( $F_MAX < $_ ){ $_ = $F_MAX; }
			$swapFrequency = $_;
		}elsif( m/^-t/i ){# CHECK FOR TABWIDTH OPTION
			if( $' eq "" ){ $_ = shift; } # value is next arg
			else{ $_ = $'; } # value is rest of this arg
			# check value is number and within range
			if( !( m/\d+/ ) ){ next; }
			if( $_ < 1 ){ $_ = 1; }
			elsif( $T_MAX < $_ ){ $_ = $T_MAX; }
			$tabWidth = $_;
		}elsif( m/^-r/i ){# CHECK FOR RIGHT MARGIN OPTION
			if( $' eq "" ){ $_ = shift; } # value is next arg
			else{ $_ = $'; } # value is rest of this arg
			# check value is number and within range
			if( !( m/\d+/ ) ){ next; }
			if( $_ < 1 ){ $_ = 1; }
			elsif( $R_MAX < $_ ){ $_ = $R_MAX; }
			$rightMargin = $_;
		}
	}
	$debugBar = "="x$rightMargin;

	# LOAD INPUT FILE
	my $input = "";
	while( <READFILE> ){
		# right trim
		s/\s+$/\n/;
		# just in case the file contains any of my marks
		s/$CURLY_MARK//g;
		s/$SWAP_MARK//g;
		# append
		$input .= $_;
	}
	close READFILE;

	# OPEN OUTPUT FILE, DIE ON ERROR
	open WRITEFILE, ">$outputFile"
		or die "\n  Output filename `$outputFile' invalid.\n";

	# DISPLAY OPTIONS CHOSEN, VERBOSE OUTPUT
	if( $verbose ){
		print "\n  Input File: `$inputFile'\n";
		print "\n  Output File: `$outputFile'\n";
		print "\n  Swap Frequency: every $swapFrequency line(s)\n";
		print "\n  Tab Width: $tabWidth\n";
		print "\n  Right Margin: $rightMargin character(s)\n";
	}

	# SPLIT LINES INTO SMALLEST TOKENS
	my @tokens = tokenizeSplit( $input );

	# MERGE CERTAIN TOKEN SEQUENCES INTO 1 LARGE TOKEN
	@tokens = tokenizeMerge( \@tokens );

	# PARSE THE TOKENS, MARK SWAPABLE LOCATIONS
	my $output = parseAndMark( \@tokens );

	# PLACE SWAPS AT THE MARKS IN A PRETTY WAY
	$output = swapAtMarks( $output );

	# WRITE OUTPUT FILE
	print WRITEFILE $output;
	close WRITEFILE;

	# VERBOSE OUTPUT
	if( $verbose ){ print "\n$debugBar\n\n  DONE :) !\n\n"; }
}
################################################################################




################################################################################
# tokenizeSplit
################################################################################
sub tokenizeSplit(){
	my $input = shift;
	my @tokens = (); # list of tokens
	my $tokenType = "";

	# DEBUG
	if( $debug ){
		# phase debug header
		print "\n\n\n\n\n$debugBar\nTOKENIZING...\n";
		printf "%6s|%6s|%s\n", "TOKEN", "TOKEN", "TOKEN";
		printf "%6s|%6s|%s\n", "NUM", "TYPE", "TEXT";
		print "$debugBar\n";
	}

	while( $_ = $input ){

		# DEBUG
		if( $debug && $#tokens >= 0 ){
			# print most recent token
			my $tok = $tokens[$#tokens];
			printf "%6d|%6s|$tok", $#tokens, $tokenType;
			if( $tok !~ m/\n$/ ){ print "|\n"; }
			$tokenType = "";
		}

		# WORD
		if( m/^\w+/ ){ $tokenType = "WORD"; push @tokens, $&; $input = $'; next; }

		# WHITESPACE
		if( m/^\n/ ){ $tokenType = "SPACE1"; push @tokens, $&; $input = $'; next; }
		if( m/^\s+?\n/ ){ $tokenType = "SPACE2"; push @tokens, $&; $input = $'; next; }
		if( m/^\s+/ ){ $tokenType = "SPACE3"; push @tokens, $&; $input = $'; next; }

		# STRING
		if( m/^"(?:\\.|[^\\"])*?"/ ){
			$tokenType = "STRING"; push @tokens, $&; $input = $'; next; }

		# CHAR LITERAL
		if( m/^'(?:\\.|[^\\'])*?'/ ){
			$tokenType = "CHARLT"; push @tokens, $&; $input = $'; next; }

		# LINE COMMENT
		if( m{^//.*?\n} ){
			$tokenType = "LINEC"; push @tokens, $&; $input = $'; next; }

		# BLOCK COMMENT
		if( m{^/\*(?:..|[^\*/])*?\*/} ){
			$tokenType = "BLOCKC"; push @tokens, $&; $input = $'; next; }

		# POUND DIRECTIVES
		if( m/^#.*?(\\\n.*?)*?\n/ ){
			$tokenType = "POUND"; push @tokens, $&; $input = $'; next; }

		# ONE CHAR (excluding dubquote ", onequote ' and pound #)
		if( m/[\/\(\)\.\,\{\}\|\+\*\^\?\[\]\$\-~`;:<>_=&%@!]/ ){
			$tokenType = "CHAR"; push @tokens, $&; $input = $'; next; }

		### SHOULD NEVER GET HERE, BUT JUST IN CASE
		push @tokens, $_;
		last;
	}
	return @tokens;
}
################################################################################





################################################################################
# tokenizeMerge
################################################################################
sub tokenizeMerge(){
	$_ = shift;
	my @inToks = @$_; # queue of input tokens
	my @outToks = (); # list of output tokens
	my $text = "";    # large token being built
	my $mergedType = "";
	# $_ is the current tok

	# DEBUG
	if( $debug ){
		# phase debug header
		print "\n\n\n\n\n$debugBar\nMERGING SPECIFIC TOKEN SEQUENCES...\n";
		printf "%6s|%6s|%s\n", "TOKEN", "MERGED", "TOKEN";
		printf "%6s|%6s|%s\n", "NUM", "TYPE", "TEXT";
		print "$debugBar\n";
	}

	# TOKENIZE, pass 2
	while( $#inToks + 1 ){ # while there are tokens
		$_ = shift @inToks; # dequeue token

		# DEBUG
		if( $debug && $#outToks >= 0 ){
			# print most recent outTok
			my $tok = $outToks[$#outToks];
			$tok =~ s/$CURLY_MARK/\@curlycount/g;
			printf "%6d|%6s|$tok", $#outToks, $mergedType;
			if( $tok !~ m/\n$/ ){ print "|\n"; }
			$mergedType = "";
		}

		# SKIP TOKENS: COMMENTS, QUOTED STRINGS, POUND DIRECTIVES
		if( m{^(/[/\*]|"|'|#)} ){ goto EVERYTHING_ELSE; }

		# PARENTHESIS BLOCKS
		if( m/^\($/ ){
			$mergedType = "PAREN";
			# build parenthesis block token starting with (
			$text = $_;
			# prepend
			if( $debug ){ print "!!! PREPEND !!!\n" }
			while( $#outToks + 1 ){
				$text = ( $_ = pop @outToks ) . $text;
				# skip comments
				if( m{^/[/\*]|^\s+$} ){ next; }
				last;
			}
			my $parenCount = 1;
			while( $#inToks + 1 ){
				# append next tok to this token
				$text .= ( $_ = shift @inToks );
				# skip comments and quoted strings
				if( m{^(/[/\*]|"|')} ){ next; }
				# token done when parenthesis are balanced (parenCount is 0)
				if( !( $parenCount += tr/\(// - tr/\)// ) ){ last; }
			}
			# append exacly like 'if'
			goto APPEND;
			next;
		}

		# ASSIGNMENT/ARRAY INITS, NOT IN PARENTHESIS
		if( m/^=$/ ){
			$mergedType = "ASSIGN";
			# build assignment token starting with =
			$text = $_;
			while( $#inToks + 1 ){
				# append next tok to this token
				$text .= ( $_ = shift @inToks );
				# token done when found semicolon
				if( m/^;$/ ){ last; }
			}
			# create token, add to tokens
			push @outToks, $text;
			next;
		}

		# DO
		if( m/^do$/){
			$mergedType = "DO";
			$text = $_;
			# append exacly like 'if'
			goto APPEND;
		}

		# WHILE
		if( m/^while$/ ){
			$text = $_;
			$mergedType = "WHILE";
			# append to this token
			while( $#inToks + 1 ){
				$text .= ( $_ = shift @inToks );
				# if found curly or semicolon, done appending
				if( m/^;$/ ){ last; }
				if( m/^{$/ ){ goto CURLY_BLOCK; }
				# if found 'for', then continue in 'for' loop code.
				if( m/^for$/ ){ goto FOR; }
			}
			# if this while has a semicolon right after the parenthesis
			if( $text =~ m/\)\s*;$/ ){
				# search outToks to determine if this 'while' belongs to a 'do'
				for( my $i = 0; $i <= $#outToks; ++$i ){
					$_ = $outToks[$#outToks-$i];
					# skip whitespace and comments
					if( m{^\s+$|^/[/\*]} ){ next; }
					# current tok must be either: a do ending with semicolon,
					if( m/;$/ && m/^do\b/ ){ $mergedType = "DO"; }
					# or a closing curly brace (note: not checking that there is a "do{" )
					elsif( m/}$/ ){ $mergedType = "END_DO"; }
					# otherwise do not prepend.
					else{ last; }
					# prepend outToks to this 'while' up to i
					if( $debug ){ print "!!! PREPEND !!!\n"; }
					for( my $j = 0; $j <= $i; ++$j ){
						$text = ( $_ = pop @outToks ) . $text; }
					last;
				}
			}
			# create token, add to tokens
			push @outToks, $text;
			next;
		}

		# ELSE
		if( m/^else$/ ){
			$text = $_;
			# prepend to this token from output tokens
			if( $debug ){ print "!!! PREPEND !!!\n" }
			while( $#outToks + 1 ){
				$text = ( $_ = pop @outToks ) . $text;
				# skip comments
				if( m{^/[/\*]} ){ next; }
				# if found curly or semicolon, done prepending
				if( m/}$/ ){ $mergedType = "ELSE"; last; }
				if( m/;$/ ){ $mergedType = "IF"; last; }
			}
			# append exacly like 'if'
			goto APPEND;
		}

		# SWITCH
		if( m/^switch$/ ){
			$mergedType = "SWITCH";
			$text = $_;
			# append exacly like 'if'
			goto APPEND;
		}

		# FOR
		if( m/^for$/ ){
			$mergedType = "FOR";
			$text = $_;
	FOR:
			foreach my $i (1,2){ # repeat 2 times
				# append to the next semicolon
				while( $#inToks + 1 ){
					$text .= ( $_ = shift @inToks );
					if( m/^;$/ ){ last; }
				}
			}
			# append exacly like 'if'
			goto APPEND;
		}

		# IF
		if( m/^if$/ ){
			$mergedType = "IF";
			$text = $_;
	APPEND:
			# append to this token
			while( $#inToks + 1 ){
				$text .= ( $_ = shift @inToks );
				# if found curly or semicolon, done appending
				if( m/^;$/ ){ last; }
				if( m/^{$/ ){ goto CURLY_BLOCK; }
				# if found 'for', then continue in 'for' loop code.
				if( m/^for$/ ){ goto FOR; }
			}
			# create token, add to tokens
			push @outToks, $text;
			next;
		}


		# CLOSE CURLY
		if( m/^}$/ ){
			$mergedType = "CLOSEC";
			# MARK THIS CURLY AS COUNTABLE FOR PARSING
			push @outToks, $CURLY_MARK . $_;
			next;
		}

		# OPEN CURLY BLOCK
		if( m/^{$/ ){
			$mergedType = "OPENC";
			$text = $_;
	CURLY_BLOCK:
			# MARK THIS CURLY AS COUNTABLE FOR PARSING
			$text .= $CURLY_MARK;
			# is 'skip variable declarations' option on?
			if( $skipVarDecls ){
				# do the following at least once
				my $repeat;
				do{ $repeat = 0;
					# peek ahead skipping whitespace, comments, pound directives
					my $i = 0;
					while( $inToks[$i] =~ m{^(/[/\*]|#)|^\s+$} ){ ++$i; }
					# check if var decl
					foreach( @VARIABLE_TYPES ){
						if( $inToks[$i] !~ m/\b$_\b/ ){ next; }
						# found var decl. Also have to check for another after this one
						$repeat = 1;
						# peek advance i to the semicolon
						while( $inToks[$i] !~ m/^;$/ ){ ++$i; }
						# append to text all toks up to and including the semicolon
						for( my $j = 0; $j <= $i; ++$j ){ $text .= shift @inToks; }
						last;
					}
				}while( $repeat );
			}
			push @outToks, $text;
			next;
		}

	EVERYTHING_ELSE:
		push @outToks, $_;
	}
	return @outToks;
}
################################################################################





################################################################################
# parseAndMark
################################################################################
sub parseAndMark(){
	$_ = shift;
	my @tokens = @$_;	# list of tokens
	my $curlyCount = 0;
	my $swapsOn = 1;
	my $lineCount = 10;
	# $_ is the current token

	# DEBUG
	if( $debug ){
		# phase debug header
		print "\n\n\n\n\n$debugBar\nPARSE AND MARK...\n";
		printf "%6s|%3s|%3s|%s\n", "TOKEN", "Ln", "{}","TOKEN";
		printf "%6s|%3s|%3s|%s\n", "NUM", "Cnt", "Cnt", "TEXT";
		print "$debugBar\n";
	}

	# PARSING
	for( my $i = 0; $i <= $#tokens; ++$i ){
		$_ = $tokens[$i];

		# DEBUG
		if( $debug ){
			# print endline for previous token
			if( $i ){ print "|\n"; }
			# print current ith token
			( my $tok = $_ ) =~ s/\n$//;
			$tok =~ s/$CURLY_MARK/\@curlycount/g;
			printf "%6d|%3d|%3d|$tok", $i, $lineCount, $curlyCount;
		}

		# NEWLINE COUNTER
		$lineCount += tr/\n//;

		# SKIP: QUOTED STRING, POUND DIRECTIVES, PAREN BLOCKS, ASSIGNS/ARRAY INITS
		if( m/^("|'|#|\(|=)/ ){ next; }

		# CURLY BRACE COUNTER
		while( m/{$CURLY_MARK/g ){ ++$curlyCount; }
		while( m/$CURLY_MARK}/g ){ --$curlyCount; }

		# SWAP CONTROL COMMENTS
		# if found a 'swaps on' directive, turn swaps on
		if( m/$SWAPS_ON/ ){ $swapsOn = 1; next; }
		# if found a 'swaps off' directive, turn swaps off
		if( m/$SWAPS_OFF/ ){ $swapsOn = 0; next; }
		# skip lines after 'swaps off' directive
		if( !$swapsOn ){ next; }

		# SKIP: COMMENTS
		if( m{^/[/\*]} ){ next; }

		# STRUCT DECLARATION
		if( m/\bstruct\b/ && $curlyCount <= 0 ){
			# advance to the closing curly brace
			do{ ++$i; }while( $tokens[$i] !~ m/$CURLY_MARK}/ );
			$lineCount += $swapFrequency;
			next;
 		}

		# CURLY BRACE COUNT CHECK
		if( $curlyCount <= 0 ){ $curlyCount = 0; next; }

		# LINES TO SKIP
		if( m/^(SWAP|swapTask|break|continue|return|longjmp|goto|exit)\b/ ){
			# advance to newline char
			do{ ++$i; }while( $tokens[$i] !~ m/\n$/ );
			++$lineCount;
			next;
		}

		# CHANCE TO ADD MARK
		# if token has newline at end and
		# its been a while since we added a SWAP to a line
		if( m/\n$/ && $lineCount >= $swapFrequency ){
			$tokens[$i] .= $SWAP_MARK;
			$lineCount = 0;
			if( $debug ){ print "\@swapmark"; }
		}
	}
	# MERGE ALL TOKENS INTO ONE LONG OUTPUT STRING
	$_ = join "", @tokens;

	# DELETE MY MARKS
	s/$CURLY_MARK//g;

	return $_;
}
################################################################################





################################################################################
# swapAtMarks
################################################################################
sub swapAtMarks(){
	# DEBUG
	if( $debug ){ print "\n\n\n\n\n$debugBar\nSWAPIFYING...\n"; }
	# split arg on SWAP_MARK into chunks, removes SWAP_MARK
	my @chunks = split /$SWAP_MARK/, ( shift );
	# endChunk didn't have a SWAP_MARK after it.
	my $endChunk = pop @chunks;
	# append a SWAP to end of each chunk, put chunks back together
	my $output = "";
	while( $#chunks + 1 ){ $output .= appendSwap( shift( @chunks ) ); }
	$output .= $endChunk;
	# DEBUG
	if( $debug ){ print $output; }
	return $output;
}
################################################################################





################################################################################
# appendSwap
# Add SWAP_CODE at end of chunk. Calculate length of last line in chunk.
# Use calculation to determine how to place SWAP at the right margin.
# All SWAPs should line up nicely to rightMargin if tabWidth is chosen correctly
################################################################################
sub appendSwap(){
	$_ = shift; s/\n$//; # grab arg and chomp
	# grab last line in the arg
	my $line = $_;
	if( /.*\n/s ){ $line = $'; }
	# count tabs in line
	my $tabCount = ( $line =~ tr/\t// );
	# determine line length
	my $lineLength = length( $line ) + ( $tabCount * ( $tabWidth - 1 ) );
	# determine char count until right margin
	my $countToEnd = $rightMargin - length( $SWAP_CODE ) - $lineLength;
	if( $countToEnd < 1 ){ $countToEnd = 3; }
	# append SWAP_CODE
	return $_ . ' ' x $countToEnd . "$SWAP_CODE\n";
}
################################################################################

