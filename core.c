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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <limits.h>
#define __USE_XOPEN
#include <time.h>
#include <regex.h>
#include <gc/gc.h>

#include "tree.h"
#include "data.h"
#include "core.h"

/* global vairable to remember data put back from reading; function "back" sets 
 * puts value here, and function "read" takes value from here */
struct vlue vlue_bufr;
int vlue_stat = 0 != 0;

/* what text has been compiled as regular expression, text of pattern, "ptxt";
 * "0" means no text has been compiled; used to keep track of text that was
 * compiled and stored as a regular expression in "pcmp" */
char *ptrn_text = 0;

/* compiled regular expression and result of last compilation; this is valid
 * only when "ptxt" is not zero */
regex_t ptrn_comp;
int ptrn_stat;

/* what text has been recently tried to match against compiled regular
 * expression; this is kept to avoid matching the same text again and again */
char *mtch_text = 0;

/* results of last regular expression matching -- starting index, ending index
 * and result of last call (matching status, "msta"); this is valid only when
 * "mtxt" is not zero */
regoff_t mtch_inds, mtch_inde;
int mtch_stat;

/* this is a tree that is accessible by "main" function */
struct tree *tree_main = 0;

/* function: if; parameters: any number of pairs boolean and value (condition,
 * value for true), and last value (when all conditions false); description:
 * evaluates condition, if true, then evaluates following expression, if false
 * then skips expression, and evaluates following condition, if all conditions
 * fail then evaluates last expression; examples: (if (yes) +1.0 +2.0) = +1.0,
 * (if (no) +1.0 +2.0) = +2.0, (if (no) +1.0 (yes) +2.0 (no) +3.0 +4.0) = +2.0,
 * (if (no) +1.0 (no) +2.0 (yes) +3.0 +4.0) = +3.0, (if (no) +1.0 (no) +2.0 (no)
 * +3.0 +4.0) = +4.0, (if +12.0) = +12.0 */
/* implemented in data.c file */

/* function: as; parameters: any number of pairs name and value, and expression 
 * to compute; description: computes values and assigns them to names, then
 * computes expression which can use these names as precomputed values;
 * examples: (as a +2.0 (mul (a) (mul (a) (a)))) = +8.0, (as a +2.0 b +3.0 (min 
 * (mul (a) (b)) (div (a) (b)))) = +0.66666666666666667, (as +4.0) = +4.0 */
/* implemented in data.c file */

/* function: do; parameters: any types, any number, at least one; description:
 * value of last parameter; examples: (do +1.0 +2.0) = +2.0 */
void core_do(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc == 0) {
		fputs("Can not compute \"do\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		vv[0] = vv[vc - 1];
	}
}

/* function: is; parameters: name and value of any type; description: tell if
 * value has a type described by name; examples: (is numb +2.0) = (yes), (is
 * numb "alfa beta") = (no) */
void core_is(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"is\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NAME) {
		fputs("Can not compute \"is\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	if (strcmp(vv[0].data.name, "void") == 0) {
		vv[0].data.bool = vv[1].type == VOID;
	} else if (strcmp(vv[0].data.name, "spac") == 0) {
		vv[0].data.bool = vv[1].type == SPAC;
	} else if (strcmp(vv[0].data.name, "tabu") == 0) {
		vv[0].data.bool = vv[1].type == TABU;
	} else if (strcmp(vv[0].data.name, "line") == 0) {
		vv[0].data.bool = vv[1].type == LINE;
	} else if (strcmp(vv[0].data.name, "lpar") == 0) {
		vv[0].data.bool = vv[1].type == LPAR;
	} else if (strcmp(vv[0].data.name, "rpar") == 0) {
		vv[0].data.bool = vv[1].type == RPAR;
	} else if (strcmp(vv[0].data.name, "numb") == 0) {
		vv[0].data.bool = vv[1].type == NUMB;
	} else if (strcmp(vv[0].data.name, "name") == 0) {
		vv[0].data.bool = vv[1].type == NAME;
	} else if (strcmp(vv[0].data.name, "text") == 0) {
		vv[0].data.bool = vv[1].type == TEXT;
	} else if (strcmp(vv[0].data.name, "bool") == 0) {
		vv[0].data.bool = vv[1].type == BOOL;
	} else if (strcmp(vv[0].data.name, "node") == 0) {
		vv[0].data.bool = vv[1].type == NODE;
	} else if (strcmp(vv[0].data.name, "tree") == 0) {
		vv[0].data.bool = vv[1].type == TREE;
	} else {
		fprintf(stderr, "Can not compute \"is\" call. Wrong name of type \"%s\".\n", vv[0].data.name);
		exit(EXIT_FAILURE);
	}
}

/* function: add; parameters: two numbers; description: sum of these numbers;
 * examples: (add +12.0 -3.0) = +9.0 */
void core_add(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"add\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB || vv[1].type != NUMB) {
		fputs("Can not compute \"add\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].data.numb = numb_norm(vv[0].data.numb + vv[1].data.numb);
}

/* function: sub; parameters: two numbers; description: difference between
 * numbers; examples: (sub +19.0 +4.0) = +15.0, (sub +19.0 -2.0) = +21.0 */
void core_sub(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"sub\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB || vv[1].type != NUMB) {
		fputs("Can not compute \"sub\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].data.numb = numb_norm(vv[0].data.numb - vv[1].data.numb);
}

/* function: mul; parameters: two numbers; description: multiplication of these 
 * numbers; examples: (mul +2.0 +3.0) = +6.0 */
void core_mul(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"mul\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB || vv[1].type != NUMB) {
		fputs("Can not compute \"mul\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].data.numb = numb_norm(vv[0].data.numb * vv[1].data.numb);
}

/* function: div; parameters: two numbers, second not zero; description:
 * division of numbers; examples: (div +3.0 +2.0) = +1.5 */
void core_div(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"div\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB || vv[1].type != NUMB) {
		fputs("Can not compute \"div\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[1].data.numb == 0.0L) {
		fputs("Can not compute \"div\" call. Division by zero.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].data.numb = numb_norm(vv[0].data.numb / vv[1].data.numb);
}

/* function: int; parameters: one number; description: number rounded to
 * integer; examples: (int +2.1) = +2.0, (int -2.9) = -3.0 */
void core_int(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"int\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB) {
		fputs("Can not compute \"int\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].data.numb = numb_norm(roundl(vv[0].data.numb));
}

/* function: flo; parameters: one number; description: biggest int not bigger
 * than number; examples: (flo +2.9) = +2.0, (flo -2.1) = -3.0 */
void core_flo(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"flo\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB) {
		fputs("Can not compute \"flo\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].data.numb = numb_norm(floorl(vv[0].data.numb));
}

/* function: cil; parameters: one number; description: smallest integer not
 * smaller than number; examples: (cil +2.1) = +3.0, (cil -2.2) = -1.0 */
void core_cil(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"cil\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB) {
		fputs("Can not compute \"cil\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].data.numb = numb_norm(ceill(vv[0].data.numb));
}

/* function: min; parameters: none or two numbers; description: mininimum
 * representable positive number for no parameters or smaller number out of
 * two; examples: (min) = +0.00000000000000001, (min +1.0 -2.9) = -2.9 */
void core_min(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc == 0) {
		vv[0].type = NUMB;
		vv[0].data.numb = numb_norm(powl(10.0L, 1 - LDBL_DIG));
	} else if (vc == 2) {
		if (vv[0].type != NUMB || vv[1].type != NUMB) {
			fputs("Can not compute \"min\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		if (vv[0].data.numb > vv[1].data.numb)
			vv[0].data.numb = vv[1].data.numb;
	} else {
		fputs("Can not compute \"min\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: max; parameters: none or two numbers; description: maximum
 * representable number or bigger number out of two; examples: (max) =
 * +99999999999999999.9, (max +1.0 -2.0) = +1.0 */
void core_max(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc == 0) {
		vv[0].type = NUMB;
		vv[0].data.numb = numb_norm(powl(10.0L, LDBL_DIG - 1) - 0.1L);
	} else if (vc == 2) {
		if (vv[0].type != NUMB || vv[1].type != NUMB) {
			fputs("Can not compute \"max\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		if (vv[0].data.numb < vv[1].data.numb)
			vv[0].data.numb = vv[1].data.numb;
	} else {
		fputs("Can not compute \"max\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: ops; parameters: one number; description: number with opposite
 * sign; examples: (ops -2.0) = +2.0, (ops 0.0) = 0.0 */
void core_ops(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"ops\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB) {
		fputs("Can not compute \"ops\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].data.numb = numb_norm(-vv[0].data.numb);
}

/* function: abs; parameters: one number; description: absolute value of
 * number; examples: (abs +1.0) = +1.0, (abs -2.9) = +2.9 */
void core_abs(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"abs\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB) {
		fputs("Can not compute \"abs\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].data.numb < 0.0L)
		vv[0].data.numb = numb_norm(-vv[0].data.numb);
}

/* function: rnd; parameters: none; description: random number in range from
 * 0.0 to +1.0; examples: (rnd) = +0.32514742745934555 */
void core_rnd(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"rnd\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = NUMB;
	vv[0].data.numb = numb_norm(drand48());
}

/* function: zer; parameters: none or one number; description: 0.0 for no
 * parameters or is number zero; examples: (zer) = 0.0, (zer +2.0) = (no), (zer 
 * 0.0) = (yes) */
void core_zer(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc == 0) {
		vv[0].type = NUMB;
		vv[0].data.numb = 0.0L;
	} else if (vc == 1) {
		if (vv[0].type != NUMB) {
			fputs("Can not compute \"zer\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].type = BOOL;
		vv[0].data.bool = vv[0].data.numb == 0.0L;
	} else {
		fputs("Can not compute \"zer\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: pos; parameters: one number; description: is number positive?;
 * examples: (pos +2.5) = (yes), (pos 0.0) = (no), (pos -2.0) = (no) */
void core_pos(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"pos\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB) {
		fputs("Can not compute \"pos\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = vv[0].data.numb > 0.0L;
}

/* function: neg; parameters: one number; description: is number negative?;
 * examples: (neg +2.0) = (no), (neg 0.0) = (no), (neg -2.0) = (yes) */
void core_neg(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"neg\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB) {
		fputs("Can not compute \"neg\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = vv[0].data.numb < 0.0L;
}

/* function: equ; parameters: two numbers; description: are numbers equal?;
 * examples: (equ +2.0 +1.0) = (no), (equ +2.0 +2.0) = (yes) */
void core_equ(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"equ\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB || vv[1].type != NUMB) {
		fputs("Can not compute \"equ\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = vv[0].data.numb == vv[1].data.numb;
}

/* function: gre; parameters: two numbers; description: is first number greater 
 * than the second one?; examples: (gre +2.0 +1.0) = (yes), (gre +1.0 +2.0) =
 * (no) */
void core_gre(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"gre\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB || vv[1].type != NUMB) {
		fputs("Can not compute \"gre\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = vv[0].data.numb > vv[1].data.numb;
}

/* function: les; parameters: two numbers; description: is first number less
 * than the second one?; examples: (les +2.0 +1.0) = (no), (les +1.0 +2.0) =
 * (yes) */
void core_les(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"les\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB || vv[1].type != NUMB) {
		fputs("Can not compute \"les\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = vv[0].data.numb < vv[1].data.numb;
}

/* function: yes; parameters: none; description: boolean value "true", "yes";
 * examples: (yes) = (yes) */
void core_yes(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"yes\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = 0 == 0;
}

/* function: no; parameters: none; description: boolean value "false", "no";
 * examples: (no) = (no) */
void core_no(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"no\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = 0 != 0;
}

/* function: not; parameters: one boolean; description: negation of boolean
 * value; examples: (not (yes)) = (no), (not (no)) = (yes) */
void core_not(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"not\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != BOOL) {
		fputs("Can not compute \"not\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = !vv[0].data.bool;
}

/* function: and; parameters: two booleans; description: logical "and" of
 * these; examples: (and (yes) (yes)) = (yes), (and (no) (yes)) = (no) */
void core_and(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"and\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != BOOL || vv[1].type != BOOL) {
		fputs("Can not compute \"and\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = vv[0].data.bool && vv[1].data.bool;
}

/* function: or; parameters: two booleans; description: logical "or" or these;
 * examples: (or (yes) (no)) = (yes), (or (no) (no)) = (no) */
void core_or(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"or\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != BOOL || vv[1].type != BOOL) {
		fputs("Can not compute \"or\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = vv[0].data.bool || vv[1].data.bool;
}

/* function: numb; parameters: one text; description: get number from text,
 * parse number from text; examples: (numb "0.0") = 0.0, (numb "-2.0") = -2.0 */
void core_numb(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"numb\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == TEXT) {
		vv[0].type = NUMB;
		vv[0].data.numb = text_numb(vv[0].data.text);
	} else {
		fputs("Can not compute \"numb\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: name; parameters: one text or one node of tree; description:
 * convert text to name or get name from node of tree; examples: (name "alfa")
 * = alfa, (name (root (ins (new) alfa +3.0))) = alfa */
void core_name(size_t vc, struct vlue *vv)
{
	char *p;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"name\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == TEXT) {
		vv[0].type = NAME;
		p = vv[0].data.text;
		if (!isalpha(*p)) {
			fputs("Can not compute \"name\" call. Wrong text format.\n", stderr);
			exit(EXIT_FAILURE);
		}
		while (*p != '\0') {
			if (*p == '_')
				p++;
			if (!isalnum(*p)) {
				fputs("Can not compute \"name\" call. Wrong text format.\n", stderr);
				exit(EXIT_FAILURE);
			}
			p++;
		}
		vv[0].data.name = vv[0].data.text;
	} else if (vv[0].type == NODE) {
		vv[0].type = NAME;
		vv[0].data.name = vv[0].data.node->name;
	} else {
		fputs("Can not compute \"name\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: text; parameters: one number or one text; description: text
 * representation of number or name; examples: (text +2.34) = "+2.34", (text
 * alfa) = "alfa" */
void core_text(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"text\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NUMB) {
		vv[0].type = TEXT;
		vv[0].data.text = numb_text(vv[0].data.numb);
	} else if (vv[0].type == NAME) {
		vv[0].type = TEXT;
		vv[0].data.text = vv[0].data.name;
	} else {
		fputs("Can not compute \"text\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: leng; parameters: one name or one text; description: length of
 * name or text, number of characters; examples: (leng alfa) = +4.0, (leng
 * "alfa beta") = +7.0 */
void core_leng(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"leng\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME) {
		vv[0].type = NUMB;
		vv[0].data.numb = strlen(vv[0].data.name);
	} else if (vv[0].type == TEXT) {
		vv[0].type = NUMB;
		vv[0].data.numb = strlen(vv[0].data.text);
	} else {
		fputs("Can not compute \"leng\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: same; parameters: two names or two texts; description: does these
 * names or text look the same?; examples: (same alfa alfa) = (yes), (same
 * "alfa" "beta") = (no) */
void core_same(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"same\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME) {
		if (vv[1].type != NAME) {
			fputs("Can not compute \"same\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].type = BOOL;
		vv[0].data.bool = strcmp(vv[0].data.name, vv[1].data.name) == 0;
	} else if (vv[0].type == TEXT) {
		if (vv[1].type != TEXT) {
			fputs("Can not compute \"same\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].type = BOOL;
		vv[0].data.bool = strcmp(vv[0].data.text, vv[1].data.text) == 0;
	} else {
		fputs("Can not compute \"same\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: bfor; parameters: two names or two texts; description: does the
 * first one sort before the second one?; examples: (bfor alfa beta) = (yes),
 * (bfor alfa aaaa) = (no) */
void core_bfor(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"bfor\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME) {
		if (vv[1].type != NAME) {
			fputs("Can not compute \"bfor\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].type = BOOL;
		vv[0].data.bool = strcmp(vv[0].data.name, vv[1].data.name) < 0;
	} else if (vv[0].type == TEXT) {
		if (vv[1].type != TEXT) {
			fputs("Can not compute \"bfor\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].type = BOOL;
		vv[0].data.bool = strcmp(vv[0].data.text, vv[1].data.text) < 0;
	} else {
		fputs("Can not compute \"bfor\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: aftr; parameters: two names or two texts; description: does the
 * first one sort after the second one; examples: (aftr alfa aaaa) = (yes),
 * (aftr alfa beta) = (no) */
void core_aftr(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"aftr\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME) {
		if (vv[1].type != NAME) {
			fputs("Can not compute \"aftr\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].type = BOOL;
		vv[0].data.bool = strcmp(vv[0].data.name, vv[1].data.name) > 0;
	} else if (vv[0].type == TEXT) {
		if (vv[1].type != TEXT) {
			fputs("Can not compute \"aftr\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].type = BOOL;
		vv[0].data.bool = strcmp(vv[0].data.text, vv[1].data.text) > 0;
	} else {
		fputs("Can not compute \"aftr\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: escp; parameters: one text; description: escape special chars in
 * text; examples: (escp "a\"b\\c") = "a\\\"b\\\\c" */
void core_escp(size_t vc, struct vlue *vv)
{
	size_t l;
	char *t, *s, *d;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"escp\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TEXT) {
		fputs("Can not compute \"escp\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	l = strlen(vv[0].data.text);
	if (l > (SIZE_MAX - 1) / 2) {
		fputs("Limit reached.\n", stderr);
		exit(EXIT_FAILURE);
	}
	t = GC_malloc(2 * l + 1);
	if (t == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	for (s = vv[0].data.text, d = t, l = 0; *s != '\0'; s++, d++, l++) {
		if (*s == '"' || *s == '\\') {
			*d = '\\';
			d++;
			l++;
		}
		*d = *s;
	}
	*d = '\0';
	vv[0].data.text = GC_realloc(t, l + 1);
}

/* function: unes; parameters: one text; description: unescape special chars in 
 * text; examples: (unes "a\\\"b\\\\c") = "a\"b\\c" */
void core_unes(size_t vc, struct vlue *vv)
{
	char *t, *s, *d;
	size_t l;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"unes\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TEXT) {
		fputs("Can not compute \"unes\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	l = strlen(vv[0].data.name);
	t = GC_malloc(l + 1);
	if (t == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	for (s = vv[0].data.text, d = t; *s != '\0'; s++, d++) {
		if (*s == '\\') {
			s++;
			if (*s != '"' && *s != '\\') {
				fputs("Can not compute \"unes\" call. Wrong escape sequence.\n", stderr);
				exit(EXIT_FAILURE);
			}
			l--;
		}
		*d = *s;
	}
	*d = '\0';
	vv[0].data.text = GC_realloc(t, l + 1);
}

/* function: uppr; parameters: one name or one text; description: convert
 * letters to uppercase; examples: (uppr Alfa) = ALFA */
void core_uppr(size_t vc, struct vlue *vv)
{
	char *s, *d;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"uppr\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME) {
		s = vv[0].data.name;
		d = GC_malloc(strlen(s) + 1);
		if (d == 0) {
			fputs("Out of memory.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].data.name = d;
	} else if (vv[0].type == TEXT) {
		s = vv[0].data.text;
		d = GC_malloc(strlen(s) + 1);
		if (d == 0) {
			fputs("Out of memory.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].data.text = d;
	} else {
		fputs("Can not compute \"uppr\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	for (; *s != '\0'; s++, d++)
		*d = toupper(*s);
	*d = '\0';
}

/* function: lowr; parameters: one name or one text; description: convert
 * letters to lowercase; examples: (lowr "ALFA beta") = "alfa beta" */
void core_lowr(size_t vc, struct vlue *vv)
{
	char *s, *d;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"lowr\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME) {
		s = vv[0].data.name;
		d = GC_malloc(strlen(s) + 1);
		if (d == 0) {
			fputs("Out of memory.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].data.name = d;
	} else if (vv[0].type == TEXT) {
		s = vv[0].data.text;
		d = GC_malloc(strlen(s) + 1);
		if (d == 0) {
			fputs("Out of memory.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].data.text = d;
	} else {
		fputs("Can not compute \"lowr\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	for (; *s != '\0'; s++, d++)
		*d = tolower(*s);
	*d = '\0';
}

/* function: head; parameters: one text or one name, and one number;
 * description: beginning of text or name; examples: (head alfa +2.0) = al,
 * (head "alfa beta" +6.0) = "alfa b" */
void core_head(size_t vc, struct vlue *vv)
{
	char *s, *d;
	size_t i, l;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"head\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME) {
		s = vv[0].data.name;
	} else if (vv[0].type == TEXT) {
		s = vv[0].data.text;
	} else {
		fputs("Can not compute \"head\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[1].type != NUMB) {
		fputs("Can not compute \"head\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (roundl(vv[1].data.numb) != vv[1].data.numb) {
		fputs("Can not compute \"head\" call. Index not integer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	l = strlen(s);
	if (vv[1].data.numb < 0 || vv[1].data.numb > l) {
		fputs("Can not compute \"head\" call. Index out of range.\n", stderr);
		exit(EXIT_FAILURE);
	}
	i = vv[1].data.numb;
	d = GC_malloc(i + 1);
	if (d == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	strncpy(d, s, i);
	d[i] = '\0';
	if (vv[0].type == NAME) {
		if (!isalpha(d[0]) || !isalnum(d[i - 1])) {
			/* function will not produce valid name as expected */
			fputs("Can not compute \"head\" call. Not a name produced.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].data.name = d;
	} else {
		vv[0].data.text = d;
	}
}

/* function: tail; parameters: one text or one name, and one number;
 * description: get ending of text or name; examples: (tail alfa +2.0) = fa,
 * (tail "alfa beta" +6.0) = "a beta" */
void core_tail(size_t vc, struct vlue *vv)
{
	char *s, *d;
	size_t i, l;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"tail\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME) {
		s = vv[0].data.name;
	} else if (vv[0].type == TEXT) {
		s = vv[0].data.text;
	} else {
		fputs("Can not compute \"tail\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (roundl(vv[1].data.numb) != vv[1].data.numb) {
		fputs("Can not compute \"tail\" call. Index not integer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	l = strlen(s);
	if (vv[1].data.numb < 0 || vv[1].data.numb > l) {
		fputs("Can not compute \"tail\" call. Index out of range.\n", stderr);
		exit(EXIT_FAILURE);
	}
	i = vv[1].data.numb;
	d = GC_malloc(i + 1);
	if (d == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	strncpy(d, s + l - i, i);
	d[i] = '\0';
	if (vv[0].type == NAME) {
		if (!isalpha(d[0]) || !isalnum(d[i - 1])) {
			/* function will not produce valid name as expected */
			fputs("Can not compute \"tail\" call. Not a name produced.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].data.name = d;
	} else {
		vv[0].data.text = d;
	}
}

/* function: join; parameters: two names or two texts; description: join names
 * with underscore, or concatenate texts; examples: (join alfa beta) =
 * alfa_beta, (join "alfa" "beta") = "alfabeta" */
void core_join(size_t vc, struct vlue *vv)
{
	size_t k, l;
	char *t;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"join\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME) {
		if (vv[1].type != NAME) {
			fputs("Can not compute \"join\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		l = strlen(vv[0].data.name);
		k = strlen(vv[1].data.name);
		if (l > SIZE_MAX - 1 - k - 1) {
			fputs("Limit reached.\n", stderr);
			exit(EXIT_FAILURE);
		}
		/* all chars from first string, underline, all chars from
		 * second string, null char */
		t = GC_malloc(l + 1 + k + 1);
		if (t == 0) {
			fputs("Out of memory.\n", stderr);
			exit(EXIT_FAILURE);
		}
		strcpy(t, vv[0].data.name);
		t[l] = '_';
		strcpy(t + l + 1, vv[1].data.name);
		vv[0].data.name = t;
	} else if (vv[0].type == TEXT) {
		if (vv[1].type != TEXT) {
			fputs("Can not compute \"join\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		if (*vv[0].data.text == '\0') {
			/* first empty, return second */
			vv[0].data.text = vv[1].data.text;
		} else if (*vv[1].data.text == '\0') {
			/* second empty, return first */
			;	/* nothing to do */
		} else {
			/* none empty, allocate memory, copy */
			l = strlen(vv[0].data.text);
			k = strlen(vv[1].data.text);
			if (l > SIZE_MAX - k - 1) {
				fputs("Limit reached.\n", stderr);
				exit(EXIT_FAILURE);
			}
			/* all chars from first string, all chars from seconds
			 * string, null char */
			t = GC_malloc(l + k + 1);
			if (t == 0) {
				fputs("Out of memory.\n", stderr);
				exit(EXIT_FAILURE);
			}
			strcpy(t, vv[0].data.text);
			strcpy(t + l, vv[1].data.text);
			vv[0].data.text = t;
		}
	} else {
		fputs("Can not compute \"join\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: part; parameters: two names or two texts; description: does first
 * one have a part as the second one; examples: (part alfa lf) = (yes), (part
 * alfa aa) = (no) */
void core_part(size_t vc, struct vlue *vv)
{
	char *a, *b;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"part\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME && vv[1].type == NAME) {
		a = vv[0].data.name;
		b = vv[1].data.name;
	} else if (vv[0].type == TEXT && vv[1].type == TEXT) {
		a = vv[0].data.text;
		b = vv[1].data.text;
	} else {
		fputs("Can not compute \"part\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = strstr(a, b) != 0;
}

/* function: indx; parameters: two names or two texts; description: get index
 * of part; examples: (indx alfa fa) = +2.0, (index alfa a) = 0.0 */
void core_indx(size_t vc, struct vlue *vv)
{
	char *a, *b, *c;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"indx\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME && vv[1].type == NAME) {
		a = vv[0].data.name;
		b = vv[1].data.name;
	} else if (vv[0].type == TEXT && vv[1].type == TEXT) {
		a = vv[0].data.text;
		b = vv[1].data.text;
	} else {
		fputs("Can not compute \"indx\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = NUMB;
	c = strstr(a, b);
	if (c == 0) {
		fputs("Can not compute \"indx\" call. Part not valid.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].data.numb = c - a;
}

/* function: ptrn; parameters: one name or one text; description: compiles name
 * or text as a regular expression and stores that compilation inside memory for
 * later matching, returns true on success and false on failure; examples: (ptrn
 * "ab*c") = (yes), (ptrn "ab*[") = (no) */
void core_ptrn(size_t vc, struct vlue *vv)
{
	char *p;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"ptrn\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME) {
		p = vv[0].data.name;
	} else if (vv[0].type == TEXT) {
		p = vv[0].data.text;
	} else {
		fputs("Can not compute \"ptrn\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* compile regular expression; skip this step if text to compile is the
	 * same as recently */
	if (vv[0].data.text != ptrn_text) {
		/* free old memory; "ptxt" is not zero */
		if (ptrn_stat == 0)
			regfree(&ptrn_comp);
		/* compile pattern */
		ptrn_stat = regcomp(&ptrn_comp, p, REG_EXTENDED);
		if (ptrn_stat == REG_ESPACE) {
			fputs("Out of memory.\n", stderr);
			exit(EXIT_FAILURE);
		}
		/* remember new pattern */
		ptrn_text = p;
		/* forget previous matching */
		mtch_text = 0;
	}
	/* return true if "ptrn_stat" is zero */
	vv[0].type = BOOL;
	vv[0].data.bool = ptrn_stat == 0;
}

/* function: mtch; parameters: one text or one name; description: tries to
 * match name or text against compiled pattern, returns boolean value as a
 * result of matching; examples: (mtch "alfa") = (yes) */
void core_mtch(size_t vc, struct vlue *vv)
{
	char *m;
	regmatch_t i;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"mtch\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NAME) {
		m = vv[0].data.name;
	} else if (vv[0].type == TEXT) {
		m = vv[0].data.text;
	} else {
		fputs("Can not compute \"mtch\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* pattern must be defined */
	if (ptrn_text == 0 || ptrn_stat != 0) {
		fputs("Can not compute \"mtch\" call. No pattern is defined.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* do matching; skip this step if recently matched string is the same
	 * as current, because result would be the same */
	if (vv[0].data.text != mtch_text) {
		mtch_stat = regexec(&ptrn_comp, m, 1, &i, 0);
		if (mtch_stat == REG_ESPACE) {
			fputs("Out of memory.\n", stderr);
			exit(EXIT_FAILURE);
		}
		/* remember what text was matched and where */
		mtch_text = m;
		mtch_inds = i.rm_so;
		mtch_inde = i.rm_eo;
	}
	/* return true if "mtch_stat" is zero */
	vv[0].type = BOOL;
	vv[0].data.bool = mtch_stat == 0;
}

/* function: msta; parameters: none; description: starting index of matched
 * text; examples: (msta) = 0.0, (msta) = +1.0 */
void core_msta(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"msta\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* pattern must be defined */
	if (ptrn_text == 0 || ptrn_stat != 0) {
		fputs("Can not compute \"msta\" call. No pattern is defined.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* match must be done */
	if (mtch_text == 0 || mtch_stat != 0) {
		fputs("Can not compute \"msta\" call. No match is defined.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* return "mtch_inds" */
	vv[0].type = NUMB;
	vv[0].data.numb = mtch_inds;
}

/* function: mend; parameters: none; description: ending index of matched text;
 * examples: (mend) = 0.0, (mend) = +1.0 */
void core_mend(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"mend\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* pattern must be defined */
	if (ptrn_text == 0 || ptrn_stat != 0) {
		fputs("Can not compute \"mend\" call. No pattern is defined.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* match must be done */
	if (mtch_text == 0 || mtch_stat != 0) {
		fputs("Can not compute \"mend\" call. No match is defined.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* return "mtch_inde" */
	vv[0].type = NUMB;
	vv[0].data.numb = mtch_inde;
}

/* function: new; parameters: none; description: new, empty tree (object);
 * examples: (new) = (tree ...) */
void core_new(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"new\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = TREE;
	vv[0].data.tree = GC_malloc(sizeof(struct tree));
	if (vv[0].data.tree == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].data.tree->root = 0;
	vv[0].data.tree->frst = 0;
	vv[0].data.tree->last = 0;
}

/* function: ref; parameters: two trees or two nodes of tree; description: does 
 * both reference the same place in memory?; examples: (ref (new) (new)) =
 * (no), (as t (new) (ref (t) (t))) = (yes) */
void core_ref(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"ref\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NODE) {
		if (vv[1].type != NODE) {
			fputs("Can not compute \"ref\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].type = BOOL;
		vv[0].data.bool = vv[0].data.node == vv[1].data.node;
	} else if (vv[0].type == TREE) {
		if (vv[1].type != TREE) {
			fputs("Can not compute \"ref\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		vv[0].type = BOOL;
		vv[0].data.bool = vv[0].data.tree == vv[1].data.tree;
	} else {
		fputs("Can not compute \"ref\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: any; parameters: one tree; description: are there any nodes in
 * tree?; examples: (any (new)) = (no), (any (ins (new) alfa +1.0)) = (yes) */
void core_any(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"any\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TREE) {
		fputs("Can not compute \"any\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = vv[0].data.tree->root != 0;
}

/* function: qty; parameters: one tree; description: number of nodes in tree;
 * examples: (qty (new)) = 0.0, (qty (ins (new) alfa +1.0)) = +1.0 */
void core_qty(size_t vc, struct vlue *vv)
{
	struct node *n;
	long double q = 0.0L;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"qty\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TREE) {
		fputs("Can not compute \"qty\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	for (n = vv[0].data.tree->frst; n != 0; n = n->next)
		q += 1.0L;
	vv[0].type = NUMB;
	vv[0].data.numb = q;
}

/* function: def; parameters: one tree and one name; description: is node of
 * given name defined in tree?; examples: (def (new) alfa) = (no), (def (ins
 * (new) alfa +1.0) alfa) = (yes) */
void core_def(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"def\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TREE || vv[1].type != NAME) {
		fputs("Can not compute \"def\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = BOOL;
	vv[0].data.bool = tree_search(vv[0].data.tree, vv[1].data.name) != 0;
}

/* function: ins; parameters: one tree, one name, and one of any type;
 * description: inserts into tree new node with given name and value, returns
 * tree, error if name already defined; examples: (ins (ins (new) alfa +1.0)
 * beta +2.0) = (tree alfa +1.0 beta +2.0) */
void core_ins(size_t vc, struct vlue *vv)
{
	struct node *n;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 3) {
		fputs("Can not compute \"ins\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TREE || vv[1].type != NAME) {
		fputs("Can not compute \"ins\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	n = tree_insert(vv[0].data.tree, vv[1].data.name);
	if (n == 0) {
		fprintf(stderr, "Can not compute \"ins\" call. Tree node \"%s\" already defined.\n", vv[1].data.name);
		exit(EXIT_FAILURE);
	}
	n->vlue = GC_malloc(sizeof(struct vlue));
	if (n->vlue == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	*(struct vlue *)n->vlue = vv[2];
}

/* function: del; parameters: one tree, one name; description: deletes from
 * tree node of given name, error if node not defined; examples: (del (ins (ins 
 * (new) alfa +1.0) beta +2.0) alfa) = (tree beta +2.0) */
void core_del(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"del\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TREE || vv[1].type != NAME) {
		fputs("Can not compute \"del\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (tree_delete(vv[0].data.tree, vv[1].data.name) == 0) {
		fprintf(stderr, "Can not compute \"del\" call. Tree node \"%s\" not defined.\n", vv[1].data.name);
		exit(EXIT_FAILURE);
	}
}

/* function: set; parameters: one tree, one name, one value of any type, or one 
 * node and one value of any type; description: in tree set node of given name
 * to given value, error if node of that name not defined, or set value of
 * given node; examples: (set (ins (new) alfa +2.0) alfa -99.0) = (tree alfa
 * -99.0), (as t (ins (new) alfa +2.0) (do (set (root (t)) +99.0) (t))) = (tree 
 * alfa +99.0) */
void core_set(size_t vc, struct vlue *vv)
{
	struct node *n;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc == 0) {
		fputs("Can not compute \"set\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NODE) {
		if (vc != 2) {
			fputs("Can not compute \"set\" call. Wrong number of parameters.\n", stderr);
			exit(EXIT_FAILURE);
		}
		n = vv[0].data.node;
		*(struct vlue *)n->vlue = vv[1];
	} else if (vv[0].type == TREE) {
		if (vc != 3) {
			fputs("Can not compute \"set\" call. Wrong number of parameters.\n", stderr);
			exit(EXIT_FAILURE);
		}
		if (vv[1].type != NAME) {
			fputs("Can not compute \"set\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		n = tree_search(vv[0].data.tree, vv[1].data.name);
		if (n == 0) {
			fprintf(stderr, "Can not compute \"set\" call. Tree node \"%s\" not defined.\n", vv[1].data.name);
			exit(EXIT_FAILURE);
		}
		*(struct vlue *)n->vlue = vv[2];
	} else {
		fputs("Can not compute \"set\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: get; parameters: one tree and one name, or one node of tree;
 * description: get value from named node in given tree, or get value of node;
 * examples: (as t (ins (new) alfa +2.0) (get (t) alfa)) = +2.0, (as t (ins
 * (new) alfa +2.0) (get (root (t)))) */
void core_get(size_t vc, struct vlue *vv)
{
	struct node *n;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc == 0) {
		fputs("Can not compute \"get\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type == NODE) {
		if (vc != 1) {
			fputs("Can not compute \"get\" call. Wrong number of parameters.\n", stderr);
			exit(EXIT_FAILURE);
		}
		n = vv[0].data.node;
	} else if (vv[0].type == TREE) {
		if (vc != 2) {
			fputs("Can not compute \"get\" call. Wrong number of parameters.\n", stderr);
			exit(EXIT_FAILURE);
		}
		if (vv[1].type != NAME) {
			fputs("Can not compute \"get\" call. Wrong type of parameter.\n", stderr);
			exit(EXIT_FAILURE);
		}
		n = tree_search(vv[0].data.tree, vv[1].data.name);
		if (n == 0) {
			fprintf(stderr, "Can not compute \"get\" call. Tree node \"%s\" not defined.\n", vv[1].data.name);
			exit(EXIT_FAILURE);
		}
	} else {
		fputs("Can not compute \"get\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0] = *(struct vlue *)n->vlue;
}

/* function: root; parameters: one tree; description: root node of tree (top of 
 * AVL tree); examples: (as t (new) (do (ins (t) a +1.0) (ins (t) b +2.0) (ins
 * (t) c +3.0) (root (t)))) = (node b +2.0) */
void core_root(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"root\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TREE) {
		fputs("Can not compute \"root\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].data.tree->root != 0) {
		vv[0].type = NODE;
		vv[0].data.node = vv[0].data.tree->root;
	} else {
		vv[0].type = VOID;
	}
}

/* function: frst; parameters: one tree; description: frst (leftmost) node of
 * AVL tree; examples: (as t (new) (do (ins (t) a +1.0) (ins (t) b +2.0) (ins
 * (t) c +3.0) (frst (t)))) = (node a +1.0) */
void core_frst(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"frst\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TREE) {
		fputs("Can not compute \"frst\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].data.tree->frst != 0) {
		vv[0].type = NODE;
		vv[0].data.node = vv[0].data.tree->frst;
	} else {
		vv[0].type = VOID;
	}
}

/* function: last; parameters: one tree; description: last (rightmost) node of
 * AVL tree; examples: (as t (new) (do (ins (t) a +1.0) (ins (t) b +2.0) (ins
 * (t) c +3.0) (last (t)))) = (node c +3.0) */
void core_last(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"last\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TREE) {
		fputs("Can not compute \"last\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].data.tree->last != 0) {
		vv[0].type = NODE;
		vv[0].data.node = vv[0].data.tree->last;
	} else {
		vv[0].type = VOID;
	}
}

/* function: left; parameters: one node of tree; description: left child of
 * node or (void) if not defined; examples: (as t (new) (do (ins (t) a +1.0)
 * (ins (t) b +2.0) (ins (t) c +3.0) (left (root (t))))) = (node a +1.0) */
void core_left(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"left\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NODE) {
		fputs("Can not compute \"left\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].data.node->left != 0) {
		vv[0].type = NODE;
		vv[0].data.node = vv[0].data.node->left;
	} else {
		vv[0].type = VOID;
	}
}

/* function: rght; parameters: one node of tree; description: right child of
 * node or (void) if not defined; examples: (as t (new) (do (ins (t) a +1.0)
 * (ins (t) b +2.0) (ins (t) c +3.0) (rght (root (t))))) = (node c +3.0) */
void core_rght(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"rght\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NODE) {
		fputs("Can not compute \"rght\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].data.node->rght != 0) {
		vv[0].type = NODE;
		vv[0].data.node = vv[0].data.node->rght;
	} else {
		vv[0].type = VOID;
	}
}

/* function: next; parameters: one node of tree; description: next node in tree 
 * after given node or (void) if current node is last one; examples: (as t
 * (new) (do (ins (t) a +1.0) (ins (t) b +2.0) (ins (t) c +3.0) (next (root
 * (t))))) = (node c +3.0) */
void core_next(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"next\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NODE) {
		fputs("Can not compute \"next\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].data.node->next != 0) {
		vv[0].type = NODE;
		vv[0].data.node = vv[0].data.node->next;
	} else {
		vv[0].type = VOID;
	}
}

/* function: prev; parameters: one node of tree; description: previous node in
 * tree before given node or (void) if current node is first one; examples: (as
 * t (new) (do (ins (t) a +1.0) (ins (t) b +2.0) (ins (t) c +3.0) (prev (root
 * (t))))) = (node a +1.0) */
void core_prev(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"prev\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NODE) {
		fputs("Can not compute \"prev\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].data.node->prev != 0) {
		vv[0].type = NODE;
		vv[0].data.node = vv[0].data.node->prev;
	} else {
		vv[0].type = VOID;
	}
}

/* function: vlue; parameters: one node of tree; description: value of given
 * node; examples: (as t (new) (do (ins (t) a +1.0) (ins (t) b +2.0) (ins (t) c 
 * +3.0) (vlue (root (t))))) = +2.0 */
void core_vlue(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"vlue\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NODE) {
		fputs("Can not compute \"vlue\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0] = *(struct vlue *)vv[0].data.node->vlue;
}

/* function: tree; parameters: one node of tree; description: tree the node
 * belongs to; examples: (as t (new) (do (ins (t) a +1.0) (ins (t) b +2.0) (ins 
 * (t) c +3.0) (tree (root (t))))) = (tree a +1.0 b +2.0 c +3.0) */
void core_tree(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"tree\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NODE) {
		fputs("Can not compute \"tree\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = TREE;
	vv[0].data.tree = vv[0].data.node->tree;
}

/* function: node; parameters: one tree and one name; description: node of tree 
 * of given name, void if node of that name not defined; examples: (as t (ins
 * (new) alfa +2.0) (node (t) alfa)) = (node alfa +2.0), (node (new) beta) =
 * (void) */
void core_node(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 2) {
		fputs("Can not compute \"node\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TREE) {
		fputs("Can not compute \"node\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[1].type != NAME) {
		fputs("Can not compute \"node\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].data.node = tree_search(vv[0].data.tree, vv[1].data.name);
	vv[0].type = vv[0].data.node != 0 ? NODE : VOID;
}

/* function: main; parameters: none; description: a tree that is defined in
 * treep at the beginning of computation, one can use this tree to store
 * information throughout all computation, this is to hold data that must be
 * accessible all the time; examples: (do (ins (main) a +1.0) (ins (main) b
 * +2.0) (ins (main) c +3.0) (main)) = (tree a +1.0 b +2.0 c +3.0) */
void core_main(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"main\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* if main tree is zero, then initialize */
	if (tree_main == 0) {
		tree_main = GC_malloc(sizeof(struct tree));
		if (tree_main == 0) {
			fputs("Out of memory.\n", stderr);
			exit(EXIT_FAILURE);
		}
		tree_main->root = 0;
		tree_main->frst = 0;
		tree_main->last = 0;
	}
	vv[0].type = TREE;
	vv[0].data.tree = tree_main;
}

/* function: read; parameters: none; description: read one piece of data from
 * stdin, data can be number, name, text, left or right parenthesis, space,
 * tabulator, new line char, or void on the end of file; examples: (read)+2.0 = 
 * +2.0, (read) +1.0 = (spac), (do (read) (read)) "beta" = "beta" */
void core_read(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"read\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vlue_stat) {
		vv[0] = vlue_bufr;
		vlue_stat = 0 != 0;
	} else {
		vv[0] = *vlue_read();
	}
}

/* function: back; parameters: one value of any type that read returns;
 * description: cancels last "read" call, treep can remember only one such
 * value; examples: (as r (read) (do (read) (back (r)) (read)))+1.0+2.0 = +1.0 */
void core_back(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"back\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vlue_stat) {
		fputs("Can not compute \"back\" call. Value already back.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vlue_stat = 0 == 0;
	vlue_bufr = vv[0];
}

/* function: wrte; parameters: one value of any type that read returns;
 * description: writes data to stdout, outputs nothing for (void); examples:
 * (wrte "Hello World!") = "Hello World!" */
void core_wrte(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"wrte\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vlue_wrte(&vv[0]);
}

/* function: dump; parameters: one text; description: writes to stdout contents 
 * of text (without quotes, unescaped); examples: (dump "Hello World!") = Hello 
 * World! */
void core_dump(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"dump\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TEXT) {
		fputs("Can not compute \"dump\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	fputs(vv[0].data.text, stdout);
	if (feof(stdout) || ferror(stdout)) {
		fputs("Output error.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: time; parameters: none; description: number of seconds since
 * epoch; examples: (time) = +1318432558.0 */
void core_time(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"time\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = NUMB;
	vv[0].data.numb = time(0);
}

/* function: repr; parameters: one number; description: ISO representation of
 * time given as number of seconds; examples: (repr (time)) = "2011-10-12
 * 17:17:43" */
void core_repr(size_t vc, struct vlue *vv)
{
	size_t n;
	time_t t;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"repr\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NUMB) {
		fputs("Can not compute \"repr\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (roundl(vv[0].data.numb) != vv[0].data.numb) {
		fputs("Can not compute \"repr\" call. Time not integer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (roundl(vv[0].data.numb) < -2147483648.0L || roundl(vv[0].data.numb) > 2147483647.0L) {
		fputs("Can not compute \"repr\" call. Time out of range.\n", stderr);
		exit(EXIT_FAILURE);
	}
	t = roundl(vv[0].data.numb);
	vv[0].type = TEXT;
	vv[0].data.text = GC_malloc(20);
	if (vv[0].data.text == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	n = strftime(vv[0].data.text, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
	if (n != 19) {
		fputs("Internal problem.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* function: pars; parameters: one text; description: parse text as ISO date
 * format and return number of seconds; examples: (pars "1999-01-01 12:34:56")
 * = +915190496.0 */
void core_pars(size_t vc, struct vlue *vv)
{
	struct tm m;
	char *c;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"pars\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != TEXT) {
		fputs("Can not compute \"pars\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* text format by regular expression */
	if (!(isdigit(vv[0].data.text[0]) && isdigit(vv[0].data.text[1]) && isdigit(vv[0].data.text[2]) && isdigit(vv[0].data.text[3]) && vv[0].data.text[4] == '-' && isdigit(vv[0].data.text[5]) && isdigit(vv[0].data.text[6]) && vv[0].data.text[7] == '-' && isdigit(vv[0].data.text[8]) && isdigit(vv[0].data.text[9]) && vv[0].data.text[10] == ' ' && isdigit(vv[0].data.text[11]) && isdigit(vv[0].data.text[12]) && vv[0].data.text[13] == ':' && isdigit(vv[0].data.text[14]) && isdigit(vv[0].data.text[15]) && vv[0].data.text[16] == ':' && isdigit(vv[0].data.text[17]) && isdigit(vv[0].data.text[18]) && vv[0].data.text[19] == '\0')) {
		fputs("Can not compute \"pars\" call. Wrong time format.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* initialize broken-time structure, parse date into it, compute number 
	 * of seconds since epoch */
	c = strptime(vv[0].data.text, "%Y-%m-%d %H:%M:%S", &m);
	if (c == 0 || *c != '\0') {
		fputs("Can not compute \"pars\" call. Wrong time format.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* know nothing about daylight saving time */
	m.tm_isdst = -1;
	/* return number of seconds */
	vv[0].type = NUMB;
	vv[0].data.numb = timelocal(&m);
}

/* function: iden; parameters: one name; description: uniqe identifier starting 
 * with given name; examples: (iden My_Id) = My_Id_20111012_172117_957613416 */
void core_iden(size_t vc, struct vlue *vv)
{
	size_t l;
	time_t n;
	struct tm *d;
	struct timespec t;
	char *i;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"iden\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != NAME) {
		fputs("Can not compute \"iden\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	l = strlen(vv[0].data.name);
	/* underline + date + underline + time + underline + nanoseconds + null 
	 * char */
	if (l > SIZE_MAX - 27) {
		fputs("Limit reached.\n", stderr);
		exit(EXIT_FAILURE);
	}
	i = GC_malloc(l + 27);
	if (i == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	time(&n);
	d = localtime(&n);
	clock_gettime(CLOCK_REALTIME, &t);
	sprintf(i, "%s_%.4d%.2d%.2d_%.2d%.2d%.2d_%.9ld", vv[0].data.name, d->tm_year + 1900, d->tm_mon + 1, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec, t.tv_nsec);
	vv[0].type = NAME;
	vv[0].data.name = i;
}

/* function: void; parameters: any number of parameters; description: value of
 * type "void", parameters are ignored; examples: (void) = (void) */
void core_void(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	(void)vc;		/* suppress unused warning */
	vv[0].type = VOID;
}

/* function: spac; parameters: none; description: value of type "spac", this is 
 * space; examples: (spac) = (spac) */
void core_spac(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"spac\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = SPAC;
}

/* function: tabu; parameters: none; description: vlue of type "tabu", this is
 * tabulator; examples: (tabu) = (tabu) */
void core_tabu(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"tabu\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = TABU;
}

/* function: line; parameters: none; description: vlue of type "line", this is
 * new line character; examples: (line) = (line) */
void core_line(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"line\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = LINE;
}

/* function: lpar; parameters: none; description: this is value of type "lpar", 
 * this is left parenthesis; examples: (lpar) = (lpar), (wrte (lpar)) = ( */
void core_lpar(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"lpar\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = LPAR;
}

/* function: rpar; parameters: none; description: value of type "rapr", this is 
 * right parenthesis; examples: (rpar) = (rpar), (wrte (rpar)) = ) */
void core_rpar(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"rpar\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = RPAR;
}

/* function: vers; parameters: none; description: version of treep; examples:
 * (vers) = "2011-10-12" */
void core_vers(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 0) {
		fputs("Can not compute \"vers\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	vv[0].type = TEXT;
	vv[0].data.name = VERSION;
}

/* function: info; parameters: any number of values of any types, including no
 * parameters at all; description: write information about values on stderr,
 * texts are output without quotes and unescaped, return last value or void for
 * no parameters; examples: (info +2.4 alfa "a\"b\\c") = +2.4alfaa"b\c */
void core_info(size_t vc, struct vlue *vv)
{
	size_t i;
	char *t;
	struct node *n;
	struct vlue *v;
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < vc; i++) {
		if (vv[i].type == VOID) {
			fputs("(void)", stderr);
		} else if (vv[i].type == SPAC) {
			fputs("(spac)", stderr);
		} else if (vv[i].type == TABU) {
			fputs("(tabu)", stderr);
		} else if (vv[i].type == LINE) {
			fputs("(line)", stderr);
		} else if (vv[i].type == LPAR) {
			fputs("(lpar)", stderr);
		} else if (vv[i].type == RPAR) {
			fputs("(rpar)", stderr);
		} else if (vv[i].type == NUMB) {
			fputs(numb_text(vv[i].data.numb), stderr);
		} else if (vv[i].type == NAME) {
			fputs(vv[i].data.name, stderr);
		} else if (vv[i].type == TEXT) {
			/* don't escape printed text */
			fputs(vv[i].data.text, stderr);
		} else if (vv[i].type == BOOL) {
			fputs(vv[i].data.bool ? "(yes)" : "(no)", stderr);
		} else if (vv[i].type == NODE) {
			fputs("(node ", stderr);
			fputs(vv[i].data.node->name, stderr);
			fputc(' ', stderr);
			v = (struct vlue *)vv[i].data.node->vlue;
			if (v->type == VOID) {
				fputs("(void)", stderr);
			} else if (v->type == SPAC) {
				fputs("(spac)", stderr);
			} else if (v->type == TABU) {
				fputs("(tabu)", stderr);
			} else if (v->type == LINE) {
				fputs("(line)", stderr);
			} else if (v->type == LPAR) {
				fputs("(lpar)", stderr);
			} else if (v->type == RPAR) {
				fputs("(rpar)", stderr);
			} else if (v->type == NUMB) {
				fputs(numb_text(v->data.numb), stderr);
			} else if (v->type == NAME) {
				fputs(v->data.name, stderr);
			} else if (v->type == TEXT) {
				fputc('"', stderr);
				for (t = v->data.text; *t != '\0'; t++) {
					if (*t == '"' || *t == '\\')
						fputc('\\', stderr);
					fputc(*t, stderr);
				}
				fputc('"', stderr);
			} else if (v->type == BOOL) {
				fputs(v->data.bool ? "(yes)" : "(no)", stderr);
			} else if (v->type == NODE) {
				fputs("(node ...)", stderr);
			} else if (v->type == TREE) {
				fputs("(tree ...)", stderr);
			} else {
				fputs("Wrong data.\n", stderr);
				exit(EXIT_FAILURE);
			}
			fputc(')', stderr);
		} else if (vv[i].type == TREE) {
			fputs("(tree", stderr);
			for (n = vv[i].data.tree->frst; n != 0; n = n->next) {
				fputc(' ', stderr);
				fputs(n->name, stderr);
				fputc(' ', stderr);
				v = (struct vlue *)n->vlue;
				if (v->type == VOID) {
					fputs("(void)", stderr);
				} else if (v->type == SPAC) {
					fputs("(spac)", stderr);
				} else if (v->type == TABU) {
					fputs("(tabu)", stderr);
				} else if (v->type == LINE) {
					fputs("(line)", stderr);
				} else if (v->type == LPAR) {
					fputs("(lpar)", stderr);
				} else if (v->type == RPAR) {
					fputs("(rpar)", stderr);
				} else if (v->type == NUMB) {
					fputs(numb_text(v->data.numb), stderr);
				} else if (v->type == NAME) {
					fputs(v->data.name, stderr);
				} else if (v->type == TEXT) {
					fputc('"', stderr);
					for (t = v->data.text; *t != '\0'; t++) {
						if (*t == '"' || *t == '\\')
							fputc('\\', stderr);
						fputc(*t, stderr);
					}
					fputc('"', stderr);
				} else if (v->type == BOOL) {
					fputs(v->data.bool ? "(yes)" : "(no)", stderr);
				} else if (v->type == NODE) {
					fputs("(node ...)", stderr);
				} else if (v->type == TREE) {
					fputs("(tree ...)", stderr);
				} else {
					fputs("Wrong data.\n", stderr);
					exit(EXIT_FAILURE);
				}
			}
			fputc(')', stderr);
		} else {
			fputs("Wrong data.\n", stderr);
			exit(EXIT_FAILURE);
		}
	}
	fputc('\n', stderr);
	if (vc == 0) {
		vv[0].type = VOID;
	} else if (vc != 1) {
		vv[0] = vv[vc - 1];
	}
}

/* function: exit; parameters: one boolean; description: exits computation with 
 * success for (yes) and failure for (no); examples: (exit (no)) = failure,
 * (exit (yes)) = success */
void core_exit(size_t vc, struct vlue *vv)
{
	if (vv == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vc != 1) {
		fputs("Can not compute \"exit\" call. Wrong number of parameters.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (vv[0].type != BOOL) {
		fputs("Can not compute \"exit\" call. Wrong type of parameter.\n", stderr);
		exit(EXIT_FAILURE);
	}
	exit(vv[0].data.bool ? EXIT_SUCCESS : EXIT_FAILURE);
}
