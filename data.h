/* 
 * Copyright 2010-2011 Leszek Dubiel <leszek@dubiel.pl>
 * 
 * This file is part of Treep.
 * 
 * Treep is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * Treep is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * Treep. If not, see <http://www.gnu.org/licenses/>.
 */

/* structure to hold values that are results of computation (evaluation of
 * function call); it holds number, name or text, or reference to tree or node
 * of tree, or boolean value (used by "if" call), or reference to function
 * definition (instead of name, to avoid searching for the same function again
 * and again) */
struct vlue {
	enum {
		NUMB, NAME, TEXT,
		BOOL,
		NODE, TREE,
		SPAC, TABU, LINE, LPAR, RPAR,
		VOID,
	} type;
	union {
		long double numb;
		char *name;
		char *text;
		int bool;
		struct node *node;
		struct tree *tree;
	} data;
};

/* expression is a name, number or text or function call; function call is made
 * of left parenthesis, name of function, zero, one or more parameters, right
 * parenthesis; function name is a name or reference to function definition */
struct expr {
	enum {
		CALL,
		VLUE,
		FUNC
	} type;
	union {
		struct expr *call;
		struct vlue *vlue;
		struct func *func;
	} data;
	struct expr *next;
};

/* function definition; each function has a name; functions are of two types --
 * built-in and user-defined; built-in function is just a pointer to a
 * hard-coded, compiled-in C function that gets table of values (parameters of
 * call), computes result and saves that result in the first element of the
 * given table; user-defined function has a list of parameters (list of names)
 * and body; body is and expression to evaluate to get result of function
 * called; body refers to parameters as if they were defined functions */
struct func {
	char *name;
	enum {
		CORE,
		USER
	} type;
	union {
		void (*core) (size_t, struct vlue *);
		struct {
			size_t parc;
			char **parv;
			struct expr *expr;
		} user;
	} body;
};

/* 
 * Numbers in treep are of type long double, so computations can use only
 * LDBL_DIG digits. Minimum and maximum numbers are
 *
 *      0.00000000000000001 = powl(10.0L, 1 - LDBL_DIG)
 *      99999999999999999.9 = powl(10.0L, LDBL_DIG - 1) - 0.1L
 *      1234567890.12345678 : 18 digits, dot, 19 chars total .
 * 
 * Every operation on numbers must truncate number do display precission by
 * using function numb_norm. Truncation is made by use of printf and scanf
 * functions. Printf results are like this: 
 *
 *      printf("%.17Lf", powl(10.0L, 1 - LDBL_DIG) * 0.5L) 
 *                                              = 0.00000000000000000
 *      printf("%.17Lf", powl(10.0L, 1 - LDBL_DIG) * 0.5L + ...) 
 *                                              = 0.00000000000000001
 *      printf("%.1Lf", powl(10.0L, LDBL_DIG - 1) - 0.1L + 0.05L) 
 *                                              = 99999999999999999.9
 *      printf("%.1Lf", powl(10.0L, LDBL_DIG - 1) - 0.1L + 0.05L + ...) 
 *                                              = 100000000000000000.0 
 *      printf("%.7Lf", powl(10.0L, LDBL_DIG - 1) - 0.1L + 0.05L) 
 *                                              = 99999999999999999.9453125
 *      printf("%.7Lf", powl(10.0L, LDBL_DIG - 1) - 0.05L)
 *                                              = 99999999999999999.9531250 .
 *
 * Every number less or equal to MIN * 0.5L is converted to zero, and every
 * number greater then MAX + 0.05L generates error. This is not perfect so we
 * wait until _Decimal64 type is fully supported in compiler.
 */

long double numb_norm(long double);	/* round to display precission */
char *numb_text(long double);	/* number to text */
long double text_numb(char *);	/* text to number */

long double numb_read();
void numb_wrte(long double);
char *name_read();
void name_wrte(char *);
char *text_read();
void text_wrte(char *);

struct vlue *vlue_read(void);
void vlue_wrte(struct vlue *);

struct expr *expr_read(void);
void expr_wrte(struct expr *);

struct func *func_read(void);
void func_wrte(struct func *);

/* value of expression; if expression is just a simple number, name or text --
 * return it; if it is a function call then evaluate it using given function
 * definitions */
struct vlue *expr_vlue(struct expr *, struct tree *);
