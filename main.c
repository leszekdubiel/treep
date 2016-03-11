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

#include <ctype.h>
#include <float.h>
#include <gc/gc.h>
#include <math.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __USE_XOPEN
#include <time.h>

#include "tree.h"
#include "data.h"
#include "core.h"

int main(void)
{
	struct func *f;
	struct tree *d;
	struct node *n;
	int c;
	size_t i;
	char b[256];		/* digits, extra digit, dot, null */
	struct timespec m;

	/* binds function in Treep with code in C */
	/* *INDENT-OFF* */
	struct bind {
		char *name;
		void (*core) (size_t, struct vlue *);
	} t[] = {

		{"do", core_do},
		{"is", core_is},

		{"add", core_add},
		{"sub", core_sub},
		{"mul", core_mul},
		{"div", core_div},
		{"int", core_int},
		{"flo", core_flo},
		{"cil", core_cil},
		{"min", core_min},
		{"max", core_max},
		{"ops", core_ops},
		{"abs", core_abs},
		{"rnd", core_rnd},

		{"zer", core_zer},
		{"pos", core_pos},
		{"neg", core_neg},
		{"equ", core_equ},
		{"les", core_les},
		{"gre", core_gre},

		{"yes", core_yes},
		{"no", core_no},
		{"not", core_not},
		{"and", core_and},
		{"or", core_or},

		{"numb", core_numb},
		{"name", core_name},
		{"text", core_text},

		{"leng", core_leng},
		{"same", core_same},
		{"bfor", core_bfor},
		{"aftr", core_aftr},
		{"escp", core_escp},
		{"unes", core_unes},
		{"uppr", core_uppr},
		{"lowr", core_lowr},
		{"head", core_head},
		{"tail", core_tail},
		{"join", core_join},
		{"part", core_part},
		{"indx", core_indx},
		{"ptrn", core_ptrn},
		{"mtch", core_mtch},
		{"msta", core_msta},
		{"mend", core_mend},

		{"new", core_new},
		{"ref", core_ref},
		{"any", core_any},
		{"qty", core_qty},
		{"def", core_def},
		{"ins", core_ins},
		{"del", core_del},
		{"set", core_set},
		{"get", core_get},

		{"root", core_root},
		{"frst", core_frst},
		{"last", core_last},
		{"left", core_left},
		{"rght", core_rght},
		{"next", core_next},
		{"prev", core_prev},
		{"vlue", core_vlue},
		{"tree", core_tree},
		{"node", core_node},
		{"main", core_main},

		{"read", core_read},
		{"back", core_back},
		{"wrte", core_wrte},
		{"dump", core_dump},

		{"void", core_void},
		{"spac", core_spac},
		{"tabu", core_tabu},
		{"line", core_line},
		{"lpar", core_lpar},
		{"rpar", core_rpar},

		{"time", core_time},
		{"repr", core_repr},
		{"pars", core_pars},

		{"iden", core_iden},

		{"vers", core_vers},
		{"info", core_info},
		{"exit", core_exit}
	};
	/* *INDENT-ON* */

	/* check if number characteristics are as expected, that is if
	 * (powl(10.0L, 1 - LDBL_DIG) * 0.5L) is zero, and (powl(10.0L,
	 * LDBL_DIG - 1) - 0.1L + 0.05L) is all nines; otherwise numerical
	 * computations are not trustworthy */
	sprintf(b, "%.17Lf", powl(10.0L, 1 - LDBL_DIG) * 0.5L);
	if (strcmp(b, "0.00000000000000000") != 0) {
		fputs("Numerical incompatibility.\n", stderr);
		exit(EXIT_FAILURE);
	}
	sprintf(b, "%.1Lf", powl(10.0L, LDBL_DIG - 1) - 0.1L + 0.05L);
	if (strcmp(b, "99999999999999999.9") != 0) {
		fputs("Numerical incompatibility.\n", stderr);
		exit(EXIT_FAILURE);
	}
	sprintf(b, "%.128Lf", powl(10.0L, 1 - LDBL_DIG) * 0.5L);
	if (strcmp(b, "0.00000000000000000499999999999999999998567943087108811759437791714243395707687996412649233946012827800586819648742675781250000000") != 0) {
		fputs("Numerical incompatibility.\n", stderr);
		exit(EXIT_FAILURE);
	}
	sprintf(b, "%.16Lf", powl(10.0L, LDBL_DIG - 1) - 0.1L + 0.05L);
	if (strcmp(b, "99999999999999999.9453125000000000") != 0) {
		fputs("Numerical incompatibility.\n", stderr);
		exit(EXIT_FAILURE);
	}

	/* seed random numbers */
	clock_gettime(CLOCK_REALTIME, &m);
	srand48(m.tv_nsec);

	/* initialize global variable to hold functions */
	d = GC_malloc(sizeof(struct tree));
	if (d == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	d->root = 0;
	d->frst = 0;
	d->last = 0;

	/* insert built-in function definitions */
	for (i = 0; i < sizeof(t) / sizeof(struct bind); i++) {
		if (strcmp(t[i].name, "if") == 0 || strcmp(t[i].name, "as") == 0) {
			fprintf(stderr, "Can not define function. Function \"%s\" already defined.\n", t[i].name);
			exit(EXIT_FAILURE);
		}
		n = tree_insert(d, t[i].name);
		if (n == 0) {
			fprintf(stderr, "Can not define function. Function \"%s\" already defined.\n", t[i].name);
			exit(EXIT_FAILURE);
		}
		f = GC_malloc(sizeof(struct func));
		if (f == 0) {
			fputs("Out of memory.\n", stderr);
			exit(EXIT_FAILURE);
		}
		f->name = t[i].name;
		f->type = CORE;
		f->body.core = t[i].core;
		n->vlue = f;
	}

	/* read input, evaluate expressions until end of file */
	c = fgetc(stdin);
	if (ferror(stdin)) {
		fputs("Input error.\n", stderr);
		exit(EXIT_FAILURE);
	}
	while (!feof(stdin)) {
		if (c == ' ' || c == '\t' || c == '\n') {
			/* skip white space */
		} else if (c == '*') {
			/* skip white space */
			c = fgetc(stdin);
			if (feof(stdin) || ferror(stdin)) {
				fputs("Input error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			while (c == ' ' || c == '\t' || c == '\n') {
				c = fgetc(stdin);
				if (feof(stdin) || ferror(stdin)) {
					fputs("Input error.\n", stderr);
					exit(EXIT_FAILURE);
				}
			}
			ungetc(c, stdin);
			if (feof(stdin) || ferror(stdin)) {
				fputs("Input error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			/* read function */
			f = func_read();
			/* insert function */
			if (strcmp(f->name, "if") == 0 || strcmp(f->name, "as") == 0) {
				fprintf(stderr, "Can not define function. Function \"%s\" already defined.\n", f->name);
				exit(EXIT_FAILURE);
			}
			n = tree_insert(d, f->name);
			if (n == 0) {
				fprintf(stderr, "Can not define function. Function \"%s\" already defined.\n", f->name);
				exit(EXIT_FAILURE);
			}
			n->vlue = f;
		} else {
			/* read and evaluate expression */
			ungetc(c, stdin);
			if (feof(stdin) || ferror(stdin)) {
				fputs("Input error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			expr_vlue(expr_read(), d);
		}
		c = fgetc(stdin);
		if (ferror(stdin)) {
			fputs("Input error.\n", stderr);
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);
}
