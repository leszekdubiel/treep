
Copyright 2010-2011 Leszek Dubiel <leszek@dubiel.pl>

This file is part of Treep.

Treep is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Treep is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Treep. If not, see <http://www.gnu.org/licenses/>.

WHAT IS TREEP?

Treep is a simple language for doing symbolical computations. It operates
on numbers and texts that can be organized in more complex objects. These
objects are lists of name-value pairs that are stored in memory as AVL
trees. It has about eighty built-in functions to operate on such data and
a way to define new functions. Treep syntax very much resembles Lisp.
Power of treep is it's simplicity.

Treep reads expressions from standard input and evaluates them until end of
file is reached.  Evaluation can make some more data be read from standard
input and some data be printed on stadard output or standard error.

INPUT SYNTAX

Every expression that is read from standard input must be a literal value,
function call or function definition. Literal value is a number, a name or
a text. This is all that can be put on program input.

Numbers in Treep are defined by sign, set of integral numbers, dot and set
of decimal numbers. All these parts are obligatory. Exception is zero, which
is defined as "0.0". For example these are valid numbers: "+45.0", "-99.123",
"+0.12". But these are not valid numbers: "-0.0" (sign not allowed for zero),
"45.0" (sign is missing), "-34" (decimal part is missing), "-34.1000" (extra
zeros at the end), "+04.56" (extra zeros at the beginning).  Program checks
if number is not bigger than allowed precission (long double in C).

Names in Treep are set of letters, digits and underscores, but first
character must be a letter, last character must not be underscore and no two
underscores could be one after another. Names must match regular expression
"^[A-Za-z](_?[A-Za-z0-9])*$". For example these are valid names: "alfa_1",
"beta_1_XP", "my_name". These are not valid names: "my-home" (minus is
neither letter, digit or underscore), "4you" (not a letter at the beginning),
"Alfa_23_" (underscore at the end), "Alfa__34" (two undercores in a row).

Text in Treep is a set of printable characters ([:print:] in C) enclosed in
double quotes. There are allowed two escape sequences: "\"" (double quote)
and "\\" (backslash).

Function call is a set of expressions enclosed in parentheses. The first
expression should evaluate to the name of function to be called. The rest of
expressions are function call parameters. Parameters are evaluated in order
from left to right. This means, that between parentheses there should be at
least one expression. These are valid function calls:

	(add +4.5 -9.0)
	(info "Hello World!")
	((evaluate_function_name) +5.6) .

It is possible to define a new function. New function definitions are made
of asterisk *, name of function, names of parameters enclosed in parentehses
and body of the function. The body is an expression to evaluate when the
function is called. Inside the body all parameters of the function are
accessible as if they were defined as separate functions.  Here is example
of function that computes average of two numbers:

	* average (x y) (div (add (x) (y)) +2.0) .

Name of function is "average" and it has two parameters named "x" and "y".
This funciton is called like this:

	(average +4.0 +6.0) .

While evaluation name "x" is assigned value "+4.0" and "y" is assigned value
"+6.0". In the body of the function parameters can be accessed as if they
were normal functions, so in example "(x)" will evaluate to "+4.0" and "(y)"
to "+6.0". Treep then add these numbers and divides them by "+2.0". Finally
whole function call has value "+5.0".

Standard input to Treep besides expressions to evaluate and definitions of new
functions could contain data that is read during expression evaluation. For
example in such input:

	(read)alfa(read)beta,

Treep will evaluate only two expressions, two calls of function read, because
function "read" will swallow name "alfa" and then "beta" from standard input.

COMPLEX DATA 

Treep has only one complex data type -- this is a list of name-value pairs
that is stored in memory as AVL binary tree for fast access. Values can also
be lists, so it is possible to read, build, analise very complex data types
such as graphs, trees, linked lists. 

There is a full set of built-in functions that operate on trees. One can
create new empty tree ("new" function), add and remove named elements
from tree ("ins", "del"), modify value of named element in tree ("set"),
retrieve value of named element ("get"). It is possible to access all elements
of tree directly ("frst" -- first element of tree, "last", "left", "rght"). 

READING, WRITING

Treep program works in a three-step loop. In the first step it skips spaces,
tabs and new lines. In the second step it reads an expression. In the third
step it evaluates that expression. It sounds very typical, but in the third
step treep can evaluate functions "read" or "write" that cause some data be
read from standard input or written to standard output. This is different
from other programs, because expressions to evaluate and data for that
expressions are all in the same data flow -- on standard input.

Function "read" reads one piece of data from standard input. This can be a
number, a name, a text, or a single character (one of space, tab, new line,
left parenthesis or right parenthesis). Function "write" wirtes data to
standard output. For example such treep input:

	(wrte (add (read) (read)))+4.0+2.0 

will cause treep to write "+6.0" on standard output. 

BUILT-IN FUNCTIONS

Treep has about eighty built-in functions. Some functions like "add" or
"sub" are evaluated ordinarly -- treep first evaluates parameters of such a
function, and then runs a code to compute the result. Other functions like
"read" or "write" change state of input and output, functions "new", "set",
"get", "ins", "del" change state of memory where data is stored. Treep has
also three very special and important functions -- these are "if", "do" and
"as" functions.

Function "if" is used to evaluate expressions conditionally. First parameter
is a condition to check. This is an expression that should have boolean
value. If value is true, then following expression is evaluated and no more
conditions are considered. When value of condition is false then following
expression is skipped and next condition is considered. Finally if there
is only one parameter left then it is used as "else expression" -- this is
evaluated if all conditions are false.

For example one can define function that computes nth Fibonacci number
like this:

	* fib (n)
		(if 
			(equ (n) 0.0) 0.0
			(equ (n) +1.0) +1.0
			(add (fib (sub (n) +1.0)) (fib (sub (n) +2.0)))
		) .

Function retunrs result that is evaluation of "if" function. Function "if"
has five parameters. First parameter is a condition "(equ (n) 0.0)" that checks
if number "n" is equal zero. If this is true, then following expression "0.0"
is evaluated -- this is the result of whole function "fib" call in a case "n"
is equal zero. If number "n" is not equal zero, then next condition "(equ (n)
+1.0)" is checked. If this condition is true, then expression "+1.0" becomes
a result. If both conditions are false, then the last expression is evaluated
which adds two numbers "(fib (sub (n) +1.0))" and "(fib (sub (n) +2.0))".

Function "as" is used to remember values by name, and thus to omit multiple
evaluation of the same expression. This is extremely important for evaluating
functions that cause side effects. For example function "read" gets data from 
input and one cannot read that data again -- after beeing read it has
to be stored somewhere. This is can be done like this: 

	* about_numbers ()
		(as
			x (read)
			y (read)
			(info 
				"For " (x) " and " (y) 
				" sum is " (add (x) (y)) 
				" and difference is " (sub (x) (y)) i
				"."
			)
		)

	(about_numbers)+2.0-4.5
	(about_numbers)+6.0+7.5 .

Treep reads and stores defintion of function "about_numbers". Then it reads
first expression "(about_numbers)". The body of function "about_numbers" is
computed. Function "as" first reads name "x", evaluates expression "(read)"
and result is stored as name "x". The same happens with name "y". Then last
parameter of function "as" is computed, where names "x" and "y" can be used as
ordinary functions. In the example parameters of function "info" are computed,
names "x" and "y" are used many times, but no more data is read from input.

Function "do" is used to evaluate many expressions, especially when they
cause side effects, by discarding some of results. Parameters of function
"do" are evaluated from left to right and the value of last parameter becomes
the value of the whole expression. Here is an example of function that divides
numbers and when division by zero would occur it prints warning to standard
error and returns zero: 

	* safe_divide (x y)
		(if
			(equ 0.0 (y)) (do (info "division by zero occured") 0.0)
			(div (x) (y))
		)

	(safe_divide +2.0 +5.0)
	(safe_divide +9.99 0.0)
	(safe_divide +4.0 -3.0) .

LEARN MORE

In directory "exmp.d" there is a set of examples that should be passed on
standard input to "treep" program. This files demonstrate Treep usage step
by step and are intended to show all treep functionality in detail. So issue a command: 

	./treep <exmp.d/099_shopping_example

and continue learning. 

Leszek Dubiel
leszek@dubiel.pl
www.dubiel.pl/treep

