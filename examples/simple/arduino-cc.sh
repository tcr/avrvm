#!/bin/bash
#
#  arduino-cc.sh - Build AVR C/C++ Programs with the Arduino Framework
#  Copyright (C) 2010  Clifford Wolf <clifford@clifford.at>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.

set -e

ARDUINO_VER=""
ARDUINO_DIR="/Applications/Arduino.app/Contents/Resources/Java"
TEMPDIR=".arduino-cc-tmp"

BOARD_CFLAGS="-mmcu=atmega328p -DF_CPU=16000000L"
CFLAGS="-fno-exceptions -ffunction-sections -fdata-sections -Os -I."
LDFLAGS="-Wl,--gc-sections -L."
LDLIBS=""

AVRDUDE_BOARDFLAGS="-p m328p -b 115200"
AVRDUDE="avrdude \$AVRDUDE_BOARDFLAGS -c arduino -P \"\$ttydev\" -v -U \"flash:w:\$hexfile\""
TERMINAL="teletype \"\$ttydev\" \$baudrate"

ttydev=""
baudrate=""
outfile=""

arduinolibs="core"

v() { echo "$*" >&2; "$@"; }

did_domething=0
help() {
	echo
	echo "Usage: $0 [ -B board_type ] [ -V arduino_ver ] [ -A ardunio_dir ] [ -T temp_dir ] [ -P tty_dev ] \\"
	echo "                 [ -X baudrate ] [ -l libname ] [ -o outfile ] {*.c|*.cc|*.cpp|*.pde|*.ino}"
	echo
	echo "Supported board types:"
	echo "    duemilanove168 ..... Arduino Diecimila, Duemilanove, or Nano w/ ATmega168"
	echo "    duemilanove328 ..... Arduino Duemilanove or Nano w/ ATmega328"
	echo "    uno ................ Arduino Uno (default)"
	echo
}

autolibs_process()
{
	for hdr in $( sed '/^ *# *include *</ ! d; s/.*<//; s/>.*//; \,/, d;' "$@" ); do
		libname=$( find "$ARDUINO_DIR/libraries/" -name "$hdr" -printf '%P\n' | sed 's,/.*,,' )
		[ ".$libname" != "." -a -d "$ARDUINO_DIR/libraries/$libname" ] || continue
		echo "$arduinolibs" | grep -wq "$libname" && continue
		arduinolibs="$arduinolibs $libname"
		autolibs_process $( find "$ARDUINO_DIR/libraries/$libname" -name '*.c' -o -name '*.cpp' )
	done
}

getopt_string="B:V:A:T:P:X:l:o:"
if ! getopt -Q -o $getopt_string -n "$0" -- "$@"; then help >&2; exit 1; fi
eval "set -- `getopt -q -o $getopt_string -n "$0" -- "$@"`"

while [ $# -gt 0 ]; do
	case "$1" in
		--)
			shift;;
		-B)
			case "$2" in
				duemilanove168)
					BOARD_CFLAGS="-mmcu=atmega168 -DF_CPU=16000000L"
					AVRDUDE_BOARDFLAGS="-p m168 -b 19200" ;;
				duemilanove328)
					BOARD_CFLAGS="-mmcu=atmega328p -DF_CPU=16000000L"
					AVRDUDE_BOARDFLAGS="-p m328p -b 57600" ;;
				uno)
					BOARD_CFLAGS="-mmcu=atmega328p -DF_CPU=16000000L"
					AVRDUDE_BOARDFLAGS="-p m328p -b 115200" ;;
				*)
					echo >&2
					echo "$0: Unkown board type \`$2'!" >&2
					help >&2
					exit 1
			esac
			shift; shift;;
		-V)
			ARDUINO_VER="$2"
			shift; shift;;
		-A)
			ARDUINO_DIR="$2"
			shift; shift;;
		-T)
			TEMPDIR="$2"
			shift; shift;;
		-P)
			ttydev="$2"
			shift; shift;;
		-X)
			baudrate="$2"
			shift; shift;;
		-l)
			arduinolibs="$arduinolibs $2"
			shift; shift;;
		-o)
			outfile="$2"
			shift; shift;;
		*.c|*.cc|*.cpp|*.pde|*.ino)
			did_domething=1
			mkdir -p "$TEMPDIR"
			if [ "$outfile" = "" ]; then
				outfile="${1%.*}"
			fi
			case "$1" in
				*.pde|*.ino)
					if [ -z "$ARDUINO_VER" ]; then
						[ -f "$ARDUINO_DIR/hardware/arduino/cores/arduino/WProgram.h" ] && ARDUINO_VER=23
						[ -f "$ARDUINO_DIR/hardware/arduino/cores/arduino/Arduino.h" ] && ARDUINO_VER=100
					fi
					autolibs_process "$@"
			esac
			for thislib in $arduinolibs; do
				libsrcdir="$ARDUINO_DIR/libraries/$thislib"
				if [ $thislib = core ]; then
					libsrcdir="$ARDUINO_DIR/hardware/arduino/cores/arduino"
					CFLAGS="$CFLAGS -I$ARDUINO_DIR/hardware/arduino/variants/standard"
				fi
				CFLAGS="$CFLAGS -I$libsrcdir"
				LDLIBS="-l$thislib $LDLIBS"
			done
			CFLAGS="$CFLAGS $BOARD_CFLAGS"
			[ -n "$ARDUINO_VER" ] && CFLAGS="$CFLAGS -DARDUINO=$ARDUINO_VER"
			for thislib in $arduinolibs; do
				libobjs=""
				libsrcdir="$ARDUINO_DIR/libraries/$thislib"
				if [ $thislib = core ]; then
					libsrcdir="$ARDUINO_DIR/hardware/arduino/cores/arduino"
					libobjs="$libobjs $TEMPDIR/cxa_pure_virtual.o"
					if [ ! -f "$TEMPDIR/cxa_pure_virtual.o" ]; then
						echo '#include <stdlib.h>' > "$TEMPDIR/cxa_pure_virtual.cc"
						echo 'extern "C" void __cxa_pure_virtual() { abort(); }' >> "$TEMPDIR/cxa_pure_virtual.cc"
						v avr-g++ -MD -g -w $CFLAGS -c -o "$TEMPDIR/cxa_pure_virtual.o" "$TEMPDIR/cxa_pure_virtual.cc"
						rm -f "$TEMPDIR/lib$thislib.a"
					fi
				fi
				for x in $( find "$libsrcdir" -name '*.c' ); do
					objfile="$TEMPDIR/lib${thislib}_$(basename "$x" .c).o"
					libobjs="$libobjs $objfile"
					if [ "$x" -nt "$objfile" ]; then
						v avr-gcc -MD -g -w $CFLAGS -c -o "$objfile" \
								-I"$libsrcdir" -I"$libsrcdir/utility" "$x"
						rm -f "$TEMPDIR/lib$thislib.a"
					fi
				done
				for x in $( find "$libsrcdir" -name '*.cpp' ); do
					objfile="$TEMPDIR/lib${thislib}_$(basename "$x" .cpp).o"
					libobjs="$libobjs $objfile"
					if [ "$x" -nt "$objfile" ]; then
						v avr-g++ -MD -g -w $CFLAGS -c -o "$objfile" \
								-I"$libsrcdir" -I"$libsrcdir/utility" "$x"
						rm -f "$TEMPDIR/lib$thislib.a"
					fi
				done
				if [ ! -f "$TEMPDIR/lib$thislib.a" ]; then
					echo "removed old $TEMPDIR/lib$thislib.a" >&2
					v avr-ar -q "$TEMPDIR/lib$thislib.a" $libobjs
					v avr-ranlib "$TEMPDIR/lib$thislib.a"
				fi
			done
			elffile="$outfile.elf"
			rm -f "$elffile"
			case "$1" in
				*.c)
					v avr-gcc -g -Wall -Wextra $CFLAGS -o "$elffile" "$@" $LDFLAGS $LDLIBS -L"$TEMPDIR/" ;;
				*.cc|*.cpp)
					v avr-g++ -g -Wall -Wextra $CFLAGS -o "$elffile" "$@" $LDFLAGS $LDLIBS -L"$TEMPDIR/" ;;
				*.pde|*.ino)
					cat > "$TEMPDIR/inoinclude.pl" <<- "EOT"
						print "#if defined(ARDUINO) && ARDUINO >= 100\n";
						print "#  include \"Arduino.h\"\n";
						print "#else\n";
						print "#  include \"WProgram.h\"\n";
						print "#endif\n";

						my $t = ";";
						while (<>) { $t .= $_; }
						$_ = "";

						# remove comments, preprocessor statements and strings
						$t =~ s/(\/\*.*?\*\/|\/\/.*?$|^\s*#.*?$)//msg;
						$t =~ s/"([^"\\]|\\.)*"//msg;
						$t =~ s/'([^'\\]|\\.)*'//msg;

						# replace all command blocks with '$;'
						while ($t =~ s/\{[^{}]*\}/\$;/g) {}
						$t =~ s/\s+\$;/\$;/g;

						# dump functions
						while ($t =~ s/;\s*((\w+\s*)+\([\w\s\*\&,]*\))\s*\$;/;/) { $_ .= "$1;\n"; }

						# dump variables
						while ($t =~ s/;\s*(\w+\s+[^();\$]*)\s*;/;/) {
							my $stmt = $1;
							next if $stmt =~ /^(typedef|static)(\s|^)/;
							$_ .= "extern " if $stmt !~ /^extern/;
							$stmt =~ s/=.*//;
							$_ .= "$stmt;\n"
						}

						print $_;

						# print "#if 0\n";
						# $t =~ s/\n+/\n/msg;
						# print $t;
						# print "#endif\n";
					EOT
					for file in "$@"; do
						case "$file" in
							*.pde|*.ino)
								cat "$file" ;;
						esac
					done | perl "$TEMPDIR/inoinclude.pl" > "$TEMPDIR/inoinclude.h"
					v avr-g++ -g -Wall -Wextra $CFLAGS -include "$TEMPDIR/inoinclude.h" \
							-o "$elffile" -x c++ "$@" $LDFLAGS $LDLIBS -L"$TEMPDIR/" ;;
			esac
			hexfile="$outfile.hex"
			rm -f "$hexfile"
			v avr-objcopy -j .text -j .data -O ihex "$elffile" "$hexfile"
			v avr-size "$elffile" "$hexfile"
			if [ -n "$ttydev" ]; then
				eval "v $AVRDUDE"
				if [ -n "$baudrate" ]; then
					eval "v $TERMINAL"
				fi
			fi
			exit 0;;
		*)
			echo >&2
			echo "$0: don't know what to do with command line argument \`$1'!" >&2
			help >&2
			exit 1
	esac
done

if [ $did_domething = 0 ]; then
	help >&2; exit 1
fi

exit 0

