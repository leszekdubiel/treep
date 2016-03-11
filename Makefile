# Copyright 2010-2011 Leszek Dubiel <leszek@dubiel.pl>
# 
# This file is part of Treep.
# 
# Treep is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
# 
# Treep is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
# 
# You should have received a copy of the GNU General Public License along with
# Treep. If not, see <http://www.gnu.org/licenses/>.

# "gc" garbage collector, "rt" to read nanoseconds; install package "libgc-dev"
# for garbage collector on Debian systems
CFLAGS=--std=gnu99 --pedantic -Werror -Wall -Wextra -DVERSION="\"$$(date +'%F %T')\"" -lm -lgc -lrt 
# -static -pthread

final: main.c data.h data.c core.h core.c tree.h tree.c
	# Hans Boehm: "People routinely use the garbage collector with -O2
	# without problems.  Certain program transformations are indeed not
	# guaranteed to be safe with the garbage collector.  In particular,
	# if the compiler transforms the code so that the only pointer to
	# an object points to an address outside the object, the object can
	# be prematurely reclaimed.  But gcc's Java support already relies
	# on that not happening, and I know of almost no such problems in
	# real code.  Unfortunately, the fact that this does not seem to be
	# a practical problem has also prevented any systematic effort to
	# eliminate such issues in gcc/gcj."
	gcc main.c data.c core.c tree.c -o treep $(CFLAGS) -O2 -g0 -s 

debug: main.c data.h data.c core.h core.c tree.h tree.c
	gcc main.c data.c core.c tree.c -o treep $(CFLAGS) -ggdb -pg -fprofile-arcs -ftest-coverage

clean: 
	rm -f treep *.gcda *.gcno *.gcov gmon.out

indent: 
	indent -linux -l999 -lc80 -sc -fca -fc1 *.c *.h
	sed -r 's/ (\* ){2,}/ * /' -i *.h *.c # error in comments
	rm *.c~ *.h~

