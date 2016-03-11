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
#include <regex.h>
#include <gc/gc.h>

#include "tree.h"
#include "data.h"

/* normalize number, round it do display precision */
long double numb_norm(long double x)
{
	long double y;
	char t[LDBL_DIG + 3];	/* sign + dot + LDBL_DIG digits + null char */
	if (fabsl(x) <= powl(10.0L, 1 - LDBL_DIG) * 0.5L)
		return 0.0L;
	if (fabsl(x) > powl(10.0L, LDBL_DIG - 1) - 0.1L + 0.05L) {
		fputs("Number out of range.\n", stderr);
		exit(EXIT_FAILURE);
	}
	sprintf(t, "%+.*Lf", LDBL_DIG - 1 - (fabsl(x) < 1.0L ? 0 : (int)log10l(floorl(fabsl(x)))), x);
	sscanf(t, "%Lf", &y);
	return y;
}

/* number to text */
char *numb_text(long double x)
{
	size_t i;
	char *t;
	if (fabsl(x) <= powl(10.0L, 1 - LDBL_DIG) * 0.5L)
		return "0.0";
	if (fabsl(x) > powl(10.0L, LDBL_DIG - 1) - 0.1L + 0.05L) {
		fputs("Can not convert number to text. Number out of range.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* allocate memory for string -- sign + dot + LDBL_DIG digits + null
	 * character; then print to this buffer; then cut extra zeros */
	t = GC_malloc(LDBL_DIG + 3);
	if (t == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	sprintf(t, "%+.*Lf", LDBL_DIG - 1 - (fabsl(x) < 1.0L ? 0 : (int)log10l(floorl(fabsl(x)))), x);
	for (i = LDBL_DIG + 2; t[i - 1] == '0' && t[i - 2] != '.'; i--)
		t[i - 1] = '\0';
	return t;
}

/* text to number */
long double text_numb(char *t)
{
	long double x;
	size_t i = 0;
	if (t == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (t[i] == '0') {
		/* zero */
		i++;
		/* dot */
		if (t[i] != '.') {
			fputs("Can not convert text to number. Dot expected.\n", stderr);
			exit(EXIT_FAILURE);
		}
		i++;
		/* zero */
		if (t[i] != '0') {
			fputs("Can not convert text to number. Zero expected.\n", stderr);
			exit(EXIT_FAILURE);
		}
		i++;
		/* check for extra digit */
		if (isdigit(t[i])) {
			fputs("Can not convert text to number. Unexpected digit.\n", stderr);
			exit(EXIT_FAILURE);
		}
	} else if (t[i] == '+' || t[i] == '-') {
		/* sign */
		i++;
		if (t[i] == '0') {
			/* zero */
			i++;
			/* dot */
			if (t[i] != '.') {
				fputs("Can not convert text to number. Dot expected.\n", stderr);
				exit(EXIT_FAILURE);
			}
			i++;
			/* set of digits, not empty, last not zero */
			do {
				while (t[i] == '0') {
					i++;
					if (i == LDBL_DIG + 3) {
						fputs("Can not convert text to number. Unexpected digit.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				if (!isdigit(t[i])) {
					fputs("Can not convert text to number. Digit expected.\n", stderr);
					exit(EXIT_FAILURE);
				}
				i++;
				if (i == LDBL_DIG + 3) {
					fputs("Can not convert text to number. Unexpected digit.\n", stderr);
					exit(EXIT_FAILURE);
				}
			} while (isdigit(t[i]));
		} else if (isdigit(t[i])) {
			/* set of digits, not empty, first not zero */
			do {
				i++;
				if (i == LDBL_DIG + 1) {
					fputs("Can not convert text to number. Unexpected digit.\n", stderr);
					exit(EXIT_FAILURE);
				}
			} while (isdigit(t[i]));
			/* dot */
			if (t[i] != '.') {
				fputs("Can not convert text to number. Dot expected.\n", stderr);
				exit(EXIT_FAILURE);
			}
			i++;
			if (i == LDBL_DIG + 2) {
				fputs("Can not convert text to number. Unexpected digit.\n", stderr);
				exit(EXIT_FAILURE);
			}
			/* first digit after dot, maybe zero */
			if (!isdigit(t[i])) {
				fputs("Can not convert text to number. Digit expected.\n", stderr);
				exit(EXIT_FAILURE);
			}
			i++;
			if (i == LDBL_DIG + 3) {
				fputs("Can not convert text to number. Unexpected digit.\n", stderr);
				exit(EXIT_FAILURE);
			}
			/* set of digits, maybe empty, last not zero */
			while (isdigit(t[i])) {
				while (t[i] == '0') {
					i++;
					if (i == LDBL_DIG + 3) {
						fputs("Can not convert text to number. Unexpected digit.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				if (!isdigit(t[i])) {
					fputs("Can not convert text to number. Digit expected.\n", stderr);
					exit(EXIT_FAILURE);
				}
				i++;
				if (i == LDBL_DIG + 3) {
					fputs("Can not convert text to number. Unexpected digit.\n", stderr);
					exit(EXIT_FAILURE);
				}
			}
		}
	} else {
		fputs("Can not convert text to number. Zero, plus or minus expected.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (t[i] != '\0') {
		fputs("Can not convert text to number. Unexpected character.\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* read number and return it */
	sscanf(t, "%Lf", &x);
	return x;
}

/* read number from stdin */
long double numb_read(void)
{
	long double x;
	char t[LDBL_DIG + 3];	/* sign + dot + LDBL_DIG digits + null char */
	size_t i = 0;
	t[i] = fgetc(stdin);
	if (feof(stdin) || ferror(stdin)) {
		fputs("Input error.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (t[i] == '0') {
		/* zero */
		i++;
		t[i] = fgetc(stdin);
		if (feof(stdin) || ferror(stdin)) {
			fputs("Input error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		/* dot */
		if (t[i] != '.') {
			fputs("Can not read number. Dot expected.\n", stderr);
			exit(EXIT_FAILURE);
		}
		i++;
		t[i] = fgetc(stdin);
		if (feof(stdin) || ferror(stdin)) {
			fputs("Input error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		/* zero */
		if (t[i] != '0') {
			fputs("Can not read number. Zero expected.\n", stderr);
			exit(EXIT_FAILURE);
		}
		i++;
		t[i] = fgetc(stdin);
		if (feof(stdin) || ferror(stdin)) {
			fputs("Input error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		/* check for extra digit */
		if (isdigit(t[i])) {
			fputs("Can not read number. Unexpected digit.\n", stderr);
			exit(EXIT_FAILURE);
		}
	} else if (t[i] == '+' || t[i] == '-') {
		/* sign */
		i++;
		t[i] = fgetc(stdin);
		if (feof(stdin) || ferror(stdin)) {
			fputs("Input error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		if (t[i] == '0') {
			/* zero */
			i++;
			t[i] = fgetc(stdin);
			if (feof(stdin) || ferror(stdin)) {
				fputs("Input error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			/* dot */
			if (t[i] != '.') {
				fputs("Can not read number. Dot expected.\n", stderr);
				exit(EXIT_FAILURE);
			}
			i++;
			t[i] = fgetc(stdin);
			if (feof(stdin) || ferror(stdin)) {
				fputs("Input error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			/* set of digits, not empty, last not zero */
			do {
				while (t[i] == '0') {
					i++;
					if (i == LDBL_DIG + 2) {
						fputs("Can not read number. Unexpected digit.\n", stderr);
						exit(EXIT_FAILURE);
					}
					t[i] = fgetc(stdin);
					if (feof(stdin) || ferror(stdin)) {
						fputs("Input error.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				if (!isdigit(t[i])) {
					fputs("Can not read number. Digit expected.\n", stderr);
					exit(EXIT_FAILURE);
				}
				i++;
				if (i == LDBL_DIG + 3) {
					fputs("Can not read number. Unexpected digit.\n", stderr);
					exit(EXIT_FAILURE);
				}
				t[i] = fgetc(stdin);
				if (feof(stdin) || ferror(stdin)) {
					fputs("Input error.\n", stderr);
					exit(EXIT_FAILURE);
				}
			} while (isdigit(t[i]));
		} else if (isdigit(t[i])) {
			/* set of digits, not empty, first not zero */
			do {
				i++;
				if (i == LDBL_DIG + 1) {
					fputs("Can not read number. Unexpected digit.\n", stderr);
					exit(EXIT_FAILURE);
				}
				t[i] = fgetc(stdin);
				if (feof(stdin) || ferror(stdin)) {
					fputs("Input error.\n", stderr);
					exit(EXIT_FAILURE);
				}
			} while (isdigit(t[i]));
			/* dot */
			if (t[i] != '.') {
				fputs("Can not read number. Dot expected.\n", stderr);
				exit(EXIT_FAILURE);
			}
			i++;
			if (i == LDBL_DIG + 2) {
				fputs("Can not read number. Unexpected digit.\n", stderr);
				exit(EXIT_FAILURE);
			}
			t[i] = fgetc(stdin);
			if (feof(stdin) || ferror(stdin)) {
				fputs("Input error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			/* first digit after dot, maybe zero */
			if (!isdigit(t[i])) {
				fputs("Can not read number. Digit expected.\n", stderr);
				exit(EXIT_FAILURE);
			}
			i++;
			if (i == LDBL_DIG + 3) {
				fputs("Can not read number. Unexpected digit.\n", stderr);
				exit(EXIT_FAILURE);
			}
			t[i] = fgetc(stdin);
			if (feof(stdin) || ferror(stdin)) {
				fputs("Input error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			/* set of digits, maybe empty, last not zero */
			while (isdigit(t[i])) {
				while (t[i] == '0') {
					i++;
					if (i == LDBL_DIG + 3) {
						fputs("Can not read number. Unexpected digit.\n", stderr);
						exit(EXIT_FAILURE);
					}
					t[i] = fgetc(stdin);
					if (feof(stdin) || ferror(stdin)) {
						fputs("Input error.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				if (!isdigit(t[i])) {
					fputs("Can not read number. Digit expected.\n", stderr);
					exit(EXIT_FAILURE);
				}
				i++;
				if (i == LDBL_DIG + 3) {
					fputs("Can not read number. Unexpected digit.\n", stderr);
					exit(EXIT_FAILURE);
				}
				t[i] = fgetc(stdin);
				if (feof(stdin) || ferror(stdin)) {
					fputs("Input error.\n", stderr);
					exit(EXIT_FAILURE);
				}
			}
		} else { 
			fputs("Can not read number. Digit expected.\n", stderr);
			exit(EXIT_FAILURE);
		}
	} else {
		fputs("Can not read number. Zero, plus or minus expected.\n", stderr);
		exit(EXIT_FAILURE);
	}
	ungetc(t[i], stdin);
	/* read number and return it */
	sscanf(t, "%Lf", &x);
	return x;
}

/* write number to stdout */
void numb_wrte(long double x)
{
	size_t i;
	char t[LDBL_DIG + 3];	/* sign + dot + LDBL_DIG digits + null char */
	if (fabsl(x) <= powl(10.0L, 1 - LDBL_DIG) * 0.5L) {
		fputs("0.0", stdout);
		if (feof(stdout) || ferror(stdout)) {
			fputs("Output error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		return;
	}
	if (fabsl(x) > powl(10.0L, LDBL_DIG - 1) - 0.1L + 0.05L) {
		fputs("Can not write number. Number out of range.\n", stderr);
		exit(EXIT_FAILURE);
	}
	sprintf(t, "%+.*Lf", LDBL_DIG - 1 - (fabsl(x) < 1.0L ? 0 : (int)log10l(floorl(fabsl(x)))), x);
	for (i = LDBL_DIG + 2; t[i - 1] == '0' && t[i - 2] != '.'; i--)
		t[i - 1] = '\0';
	fputs(t, stdout);
	if (feof(stdout) || ferror(stdout)) {
		fputs("Output error.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* read name from stdin */
char *name_read(void)
{
	size_t i = 0, l = 4096;
	char *b;
	int c;

	b = GC_malloc(l);
	if (b == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}

	c = fgetc(stdin);
	if (feof(stdin) || ferror(stdin)) {
		fputs("Input error.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (!isalpha(c)) {
		fputs("Can not read name. Letter expected.\n", stderr);
		exit(EXIT_FAILURE);
	}
	b[i] = c;
	i++;

	c = fgetc(stdin);
	if (feof(stdin) || ferror(stdin)) {
		fputs("Input error.\n", stderr);
		exit(EXIT_FAILURE);
	}
	while (c == '_' || isalnum(c)) {

		if (c == '_') {
			if (i == l) {
				if (l > SIZE_MAX / 2) {
					fputs("Limit reached.\n", stderr);
					exit(EXIT_FAILURE);
				}
				l *= 2;
				b = GC_realloc(b, l);
				if (b == 0) {
					fputs("Out of memory.\n", stderr);
					exit(EXIT_FAILURE);
				}
			}
			b[i] = c;
			i++;
			c = fgetc(stdin);
			if (feof(stdin) || ferror(stdin)) {
				fputs("Input error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			if (!isalnum(c)) {
				fputs("Can not read name. Letter or digit expected.\n", stderr);
				exit(EXIT_FAILURE);
			}
		}

		if (i == l) {
			if (l > SIZE_MAX / 2) {
				fputs("Limit reached.\n", stderr);
				exit(EXIT_FAILURE);
			}
			l *= 2;
			b = GC_realloc(b, l);
			if (b == 0) {
				fputs("Out of memory.\n", stderr);
				exit(EXIT_FAILURE);
			}
		}
		b[i] = c;
		i++;

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

	if (i == SIZE_MAX) {
		fputs("Limit reached.\n", stderr);
		exit(EXIT_FAILURE);
	}
	b = GC_realloc(b, i + 1);
	if (b == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	b[i] = '\0';

	return b;
}

/* write name to stdout */
void name_wrte(char *b)
{
	if (b == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (!isalpha(*b)) {
		fputs("Can not write name. Letter expected.\n", stderr);
		exit(EXIT_FAILURE);
	}
	while (*b != '\0') {
		if (*b == '_') {
			fputc(*b, stdout);
			if (feof(stdout) || ferror(stdout)) {
				fputs("Output error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			b++;
		}
		if (!isalnum(*b)) {
			fputs("Can not write name. Letter or digit expected.\n", stderr);
			exit(EXIT_FAILURE);
		}
		fputc(*b, stdout);
		if (feof(stdout) || ferror(stdout)) {
			fputs("Output error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		b++;
	}
}

/* read text from stdin */
char *text_read(void)
{
	size_t i = 0, l = 4096;
	char *b;
	int c;

	b = GC_malloc(l);
	if (b == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}

	c = fgetc(stdin);
	if (feof(stdin) || ferror(stdin)) {
		fputs("Input error.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (c != '"') {
		fputs("Can not read text. Quotation expected.\n", stderr);
		exit(EXIT_FAILURE);
	}

	c = fgetc(stdin);
	if (feof(stdin) || ferror(stdin)) {
		fputs("Input error.\n", stderr);
		exit(EXIT_FAILURE);
	}
	while (c != '"') {
		if (!isprint(c)) {
			fputs("Can not read text. Nonprintable character.\n", stderr);
			exit(EXIT_FAILURE);
		}
		if (c == '\\') {
			c = fgetc(stdin);
			if (feof(stdin) || ferror(stdin)) {
				fputs("Input error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			if (c != '"' && c != '\\') {
				fputs("Can not read text. Wrong escape sequence.\n", stderr);
				exit(EXIT_FAILURE);
			}
		}
		if (i == l) {
			if (l > SIZE_MAX / 2) {
				fputs("Limit reached.\n", stderr);
				exit(EXIT_FAILURE);
			}
			l *= 2;
			b = GC_realloc(b, l);
			if (b == 0) {
				fputs("Out of memory.\n", stderr);
				exit(EXIT_FAILURE);
			}
		}
		b[i] = c;
		i++;
		c = fgetc(stdin);
		if (feof(stdin) || ferror(stdin)) {
			fputs("Input error.\n", stderr);
			exit(EXIT_FAILURE);
		}
	}

	if (i == SIZE_MAX) {
		fputs("Limit reached.\n", stderr);
		exit(EXIT_FAILURE);
	}
	b = GC_realloc(b, i + 1);
	if (b == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	b[i] = '\0';

	return b;
}

/* write text to stdout */
void text_wrte(char *b)
{
	if (b == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	fputc('"', stdout);
	if (feof(stdout) || ferror(stdout)) {
		fputs("Output error.\n", stderr);
		exit(EXIT_FAILURE);
	}
	while (*b != '\0') {
		if (!isprint(*b)) {
			fputs("Can not write text. Nonprintable character.\n", stderr);
			exit(EXIT_FAILURE);
		}
		if (*b == '"' || *b == '\\') {
			fputc('\\', stdout);
			if (feof(stdout) || ferror(stdout)) {
				fputs("Output error.\n", stderr);
				exit(EXIT_FAILURE);
			}
		}
		fputc(*b, stdout);
		if (feof(stdout) || ferror(stdout)) {
			fputs("Output error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		b++;
	}
	fputc('"', stdout);
	if (feof(stdout) || ferror(stdout)) {
		fputs("Output error.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* read value from stdin */
struct vlue *vlue_read(void)
{
	struct vlue *v;
	int c;

	v = GC_malloc(sizeof(struct vlue));
	if (v == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}

	c = fgetc(stdin);
	if (ferror(stdin)) {
		fputs("Input error.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (feof(stdin)) {
		v->type = VOID;
	} else if (c == ' ') {
		v->type = SPAC;
	} else if (c == '\t') {
		v->type = TABU;
	} else if (c == '\n') {
		v->type = LINE;
	} else if (c == '(') {
		v->type = LPAR;
	} else if (c == ')') {
		v->type = RPAR;
	} else if (c == '0' || c == '+' || c == '-') {
		ungetc(c, stdin);
		if (feof(stdin) || ferror(stdin)) {
			fputs("Input error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		v->type = NUMB;
		v->data.numb = numb_read();
	} else if (isalpha(c)) {
		ungetc(c, stdin);
		if (feof(stdin) || ferror(stdin)) {
			fputs("Input error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		v->type = NAME;
		v->data.name = name_read();
	} else if (c == '"') {
		ungetc(c, stdin);
		if (feof(stdin) || ferror(stdin)) {
			fputs("Input error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		v->type = TEXT;
		v->data.text = text_read();
	} else {
		fputs("Can not read value. Unexpected character.\n", stderr);
		exit(EXIT_FAILURE);
	}

	return v;
}

/* write value to stdout */
void vlue_wrte(struct vlue *v)
{
	if (v->type == VOID) {
		/* write nothing */
	} else if (v->type == SPAC) {
		fputc(' ', stdout);
		if (feof(stdout) || ferror(stdout)) {
			fputs("Output error.\n", stderr);
			exit(EXIT_FAILURE);
		}
	} else if (v->type == TABU) {
		fputc('\t', stdout);
		if (feof(stdout) || ferror(stdout)) {
			fputs("Output error.\n", stderr);
			exit(EXIT_FAILURE);
		}
	} else if (v->type == LINE) {
		fputc('\n', stdout);
		if (feof(stdout) || ferror(stdout)) {
			fputs("Output error.\n", stderr);
			exit(EXIT_FAILURE);
		}
	} else if (v->type == LPAR) {
		fputc('(', stdout);
		if (feof(stdout) || ferror(stdout)) {
			fputs("Output error.\n", stderr);
			exit(EXIT_FAILURE);
		}
	} else if (v->type == RPAR) {
		fputc(')', stdout);
		if (feof(stdout) || ferror(stdout)) {
			fputs("Output error.\n", stderr);
			exit(EXIT_FAILURE);
		}
	} else if (v->type == NUMB) {
		numb_wrte(v->data.numb);
	} else if (v->type == NAME) {
		name_wrte(v->data.name);
	} else if (v->type == TEXT) {
		text_wrte(v->data.text);
	} else if (v->type == BOOL || v->type == TREE || v->type == NODE) {
		fputs("Can not write value. Wrong type.\n", stderr);
		exit(EXIT_FAILURE);
	} else {
		fputs("Wrong data.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

/* read expression from stdin */
struct expr *expr_read(void)
{
	/* ev (vector), es (size), et (top) is a stack that keeps track where
	 * to read expression */
	struct expr *expr, ***ev;
	size_t es = 4096, et = 0;
	/* lv tells the the number of nested function calls */
	unsigned int lv = 0;
	int c;

	ev = GC_malloc(es * sizeof(struct expr **));
	if (ev == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	ev[et] = &expr;
	et++;

	while (et != 0) {

		c = fgetc(stdin);
		if (feof(stdin) || ferror(stdin)) {
			fputs("Input error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		if (lv != 0)
			while (c == ' ' || c == '\t' || c == '\n') {
				c = fgetc(stdin);
				if (feof(stdin) || ferror(stdin)) {
					fputs("Input error.\n", stderr);
					exit(EXIT_FAILURE);
				}
			}

		if (c == '(') {
			*ev[et - 1] = GC_malloc(sizeof(struct expr));
			if (*ev[et - 1] == 0) {
				fputs("Out of memory.\n", stderr);
				exit(EXIT_FAILURE);
			}
			(*ev[et - 1])->type = CALL;

			if (lv == 0) {
				(*ev[et - 1])->next = 0;
				ev[et - 1] = &(*ev[et - 1])->data.call;
			} else {
				if (et == es) {
					if (es > SIZE_MAX / 2 / sizeof(struct expr **)) {
						fputs("Limit reached.\n", stderr);
						exit(EXIT_FAILURE);
					}
					es *= 2;
					ev = GC_realloc(ev, es * sizeof(struct expr **));
					if (ev == 0) {
						fputs("Out of memory.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				ev[et] = &(*ev[et - 1])->data.call;
				ev[et - 1] = &(*ev[et - 1])->next;
				et++;
			}
			if (lv == UINT_MAX) {
				fputs("Limit reached.\n", stderr);
				exit(EXIT_FAILURE);
			}
			lv++;

			/* check if name of function is next */
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
			if (!isalpha(c)) {
				fputs("Can not read expression. Name of function was expected.\n", stderr);
				exit(EXIT_FAILURE);
			}
			ungetc(c, stdin);
			if (feof(stdin) || ferror(stdin)) {
				fputs("Input error.\n", stderr);
				exit(EXIT_FAILURE);
			}

		} else if (c == ')') {
			*ev[et - 1] = 0;
			et--;
			if (lv == 0) {
				fputs("Can not read expression. Unexpected parenthesis.\n", stderr);
				exit(EXIT_FAILURE);
			}
			lv--;

		} else if (c == '0' || c == '+' || c == '-' || isalpha(c) || c == '"') {
			ungetc(c, stdin);
			if (feof(stdin) || ferror(stdin)) {
				fputs("Input error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			*ev[et - 1] = GC_malloc(sizeof(struct expr));
			if (*ev[et - 1] == 0) {
				fputs("Out of memory.\n", stderr);
				exit(EXIT_FAILURE);
			}
			(*ev[et - 1])->type = VLUE;
			(*ev[et - 1])->data.vlue = vlue_read();
			if (lv == 0) {
				(*ev[et - 1])->next = 0;
				et--;
			} else {
				ev[et - 1] = &(*ev[et - 1])->next;
			}

		} else {
			fputs("Can not read expression. Unexpected character.\n", stderr);
			exit(EXIT_FAILURE);
		}
	}

	return expr;
}

/* write expression to stdout */
void expr_wrte(struct expr *e)
{
	/* ev (vector), es (size) and et (top) is a stack that points what to
	 * write */
	struct expr **ev;
	size_t es = 4096, et = 0;
	/* lv (level) tells how many calls were nested */
	unsigned int lv = 0;

	if (e == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}

	ev = GC_malloc(es * sizeof(struct expr *));
	if (ev == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	ev[et] = e;
	et++;

	while (et != 0) {

		if (ev[et - 1] == 0) {
			fputc(')', stdout);
			if (feof(stdout) || ferror(stdout)) {
				fputs("Output error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			et--;
			lv--;
			if (lv != 0 && ev[et - 1] != 0) {
				fputc(' ', stdout);
				if (feof(stdout) || ferror(stdout)) {
					fputs("Output error.\n", stderr);
					exit(EXIT_FAILURE);
				}
			}

		} else if (ev[et - 1]->type == CALL) {
			fputc('(', stdout);
			if (feof(stdout) || ferror(stdout)) {
				fputs("Output error.\n", stderr);
				exit(EXIT_FAILURE);
			}
			name_wrte(ev[et - 1]->data.call->type == FUNC ? ev[et - 1]->data.call->data.func->name : ev[et - 1]->data.call->data.vlue->data.name);
			if (ev[et - 1]->data.call->next != 0) {
				fputc(' ', stdout);
				if (feof(stdout) || ferror(stdout)) {
					fputs("Output error.\n", stderr);
					exit(EXIT_FAILURE);
				}
			}
			if (lv == 0) {
				ev[et - 1] = ev[et - 1]->data.call->next;
			} else {
				if (et == es) {
					if (es > SIZE_MAX / 2 / sizeof(struct expr *)) {
						fputs("Limit reached.\n", stderr);
						exit(EXIT_FAILURE);
					}
					es *= 2;
					ev = GC_realloc(ev, es * sizeof(struct expr *));
					if (ev == 0) {
						fputs("Out of memory.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				ev[et] = ev[et - 1]->data.call->next;
				ev[et - 1] = ev[et - 1]->next;
				et++;
			}
			if (lv == UINT_MAX) {
				fputs("Limit reached.\n", stderr);
				exit(EXIT_FAILURE);
			}
			lv++;

		} else if (ev[et - 1]->type == VLUE) {
			vlue_wrte(ev[et - 1]->data.vlue);
			if (lv != 0) {
				ev[et - 1] = ev[et - 1]->next;
				if (ev[et - 1] != 0) {
					fputc(' ', stdout);
					if (feof(stdout) || ferror(stdout)) {
						fputs("Output error.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
			} else {
				et--;
			}

		} else {
			fputs("Wrong data.\n", stderr);
			exit(EXIT_FAILURE);
		}
	}
}

/* read function from stdin */
struct func *func_read(void)
{
	struct func *f;
	size_t i;
	int c;

	f = GC_malloc(sizeof(struct func));
	if (f == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	f->type = USER;

	f->name = name_read();

	f->body.user.parc = 0;
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
	if (c != '(') {
		fputs("Can not read function. Parenthesis expected.\n", stderr);
		exit(EXIT_FAILURE);
	}
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
	while (c != ')') {
		ungetc(c, stdin);
		if (feof(stdin) || ferror(stdin)) {
			fputs("Input error.\n", stderr);
			exit(EXIT_FAILURE);
		}
		if (f->body.user.parc == SIZE_MAX) {
			fputs("Limit reached.\n", stderr);
			exit(EXIT_FAILURE);
		}
		f->body.user.parc++;
		if (f->body.user.parc == 1) {
			f->body.user.parv = GC_malloc(sizeof(char *));
		} else {
			if (f->body.user.parc > SIZE_MAX / sizeof(char *)) {
				fputs("Limit reached.\n", stderr);
				exit(EXIT_FAILURE);
			}
			f->body.user.parv = GC_realloc(f->body.user.parv, f->body.user.parc * sizeof(char *));
		}
		if (f->body.user.parv == 0) {
			fputs("Out of memory.\n", stderr);
			exit(EXIT_FAILURE);
		}
		f->body.user.parv[f->body.user.parc - 1] = name_read();
		for (i = 0; i < f->body.user.parc - 1; i++)
			if (strcmp(f->body.user.parv[i], f->body.user.parv[f->body.user.parc - 1]) == 0)
				break;
		if (i != f->body.user.parc - 1) {
			fprintf(stderr, "Can not read function \"%s\". Duplicate parameter \"%s\".\n", f->name, f->body.user.parv[i]);
			exit(EXIT_FAILURE);
		}
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
	}

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
	f->body.user.expr = expr_read();

	return f;
}

/* write function to stdout */
void func_wrte(struct func *f)
{
	size_t i;
	if (f == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}
	name_wrte(f->name);
	fputs(" (", stdout);
	if (feof(stdout) || ferror(stdout)) {
		fputs("Output error.\n", stderr);
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < f->body.user.parc; i++) {
		name_wrte(f->body.user.parv[i]);
		if (i + 1 < f->body.user.parc) {
			fputc(' ', stdout);
			if (feof(stdout) || ferror(stdout)) {
				fputs("Output error.\n", stderr);
				exit(EXIT_FAILURE);
			}
		}
	}
	fputs(") ", stdout);
	if (feof(stdout) || ferror(stdout)) {
		fputs("Output error.\n", stderr);
		exit(EXIT_FAILURE);
	}
	expr_wrte(f->body.user.expr);
}

/* compute value of expression */
struct vlue *expr_vlue(struct expr *e, struct tree *d)
{
	/* stack for results, holds values; "vv" -- vlue vector, "vs" -- vlue
	 * size, "vt" -- vlue top; "vt" is the index of first empty element in
	 * "vv"; if "vt" is equal "vs", then "vv" must be resized in order to
	 * store value in "vv[vt]" */
	struct vlue *vv = 0;
	size_t vs = 4096, vt = 0;
	/* stack that tracks defined symbols; each symbol is a name-value pair;
	 * this is used to assign names of function parameters to computed
	 * values and also to compute "as" function call */
	struct symb {
		/* a place where name of symbol is stored */
		char *name;
		/* offset in "vv" array where is the value of symbol */
		size_t vlue;
		/* to chain symbols into linked-lists; such lists are used when
		 * searching for value of named symbol; this is offset in "sv"
		 * array where next symbol in list is located; last symbol in
		 * such list must have this field set to "SIZE_MAX" */
		size_t next;
	} *sv;
	size_t ss = 4096, st = 0;
	/* stack that points expressions to compute; when expression to
	 * evaluate is of type "VLUE", then it is immediately returned as a
	 * result, so this structure holds only information about function
	 * calls, that is expressions of type "CALL" */
	struct todo {
		/* a pointer to an expression that is a name of called
		 * function; type of this expression is "VLUE" if a pointer is
		 * to value that holds name of function, or type is "FUNC" if
		 * there is a pointer to function definition; this is set at
		 * the beginning of computation */
		struct expr *func;
		/* to track progress in computing function call -- compute
		 * parameters, defined symbols, execute body of function, check
		 * condition in "if" call or define symbol in "as" call; must be
		 * set to zero at the beginning */
		unsigned int step;
		/* pointer to expression that would be computed next, a pointer
		 * to expression that represents next parameter of call; at the
		 * beginning it is set to an expression folowing function name,
		 * to an expression for the first parameter */
		struct expr *expr;
		/* offset in "vv", where is placed value of first function call
		 * parameter; at the beginning it is set to "vt"; difference
		 * between "vt" and "parm" is the number of parameters evaluated
		 * in function call */
		size_t parm;
		/* where to start symbol lookup; when parameters of function
		 * call are already computed and stored on "vv" stack, then
		 * they are assigned names in "sv" table; this is offset of
		 * named value in "sv" table that should be checked first; at
		 * the beginning this must be set to "SIZE_MAX", what indicates
		 * there are no symbols defined */
		size_t symb;
		/* offset in "vv" stack, where to put result of function call;
		 * at the end "vv[retv]" would contain value that is the result 
		 * of function call and "vt" would be set to "vv[retv]" */
		size_t retv;
		/* offset in "sv" stack that would be set at the end of
		 * computation of function call */
		size_t rets;
	} *tv;
	size_t ts = 4096, tt = 0;
	struct node *n;
	struct func *f;
	char *a;
	size_t i;

	if (e == 0 || d == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}

	/* if expression to compute is of type "VLUE" there is no need to
	 * compute anything -- just return that value */
	if (e->type == VLUE)
		return e->data.vlue;

	/* allocate stacks; treep would be 40% percent faster if stacks were
	 * allocated as global fixed-sized arrays, but I don't know why; anyway
	 * I want treep to be scallable, so I choose dynamically allocated
	 * structures that are slower; and this would be hard to keep such
	 * static arrays because garbage collector would keep all allocated
	 * memory referenced in such arrays unless they are zeroed */
	vv = GC_malloc(vs * sizeof(struct vlue));
	if (vv == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	sv = GC_malloc(ss * sizeof(struct symb));
	if (sv == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	tv = GC_malloc(ts * sizeof(struct todo));
	if (tv == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}

	/* expression to compute has type "CALL"; "e->data.call" points to a
	 * value, that is a name of function or a function definition; remember
	 * name of function and let compute first of parameters */
	tv[tt].func = e->data.call;
	tv[tt].step = 0;
	tv[tt].expr = tv[tt].func->next;
	tv[tt].parm = 0;	/* vt == 0 */
	tv[tt].symb = SIZE_MAX;	/* no symbols defined */
	tv[tt].retv = 0;	/* vt == 0 */
	tv[tt].rets = 0;	/* st == 0 */
	tt++;

	while (tt != 0) {
		/* expression to compute has type "CALL"; "VLUE" is not allowed
		 * at the beginning of loop; "tv[tt - 1].func" is an expression
		 * that describes called function -- it is name of function or
		 * definition of function */
 start:
		/* if "tv[tt - 1].func->type" is "FUNC", then function beeing
		 * computed is something built-in or user-defined; this happen
		 * most often so this is at the beginning of loop; algorithm is
		 * to compute all parameters in "vv" stack, and then evaluate
		 * falue by calling "core" function or expression in
		 * user-defined function */
		if (tv[tt - 1].func->type == FUNC) {
			if (tv[tt - 1].step == 0) {
				/* in step "0" parameters of call are beeing
				 * computed and stored on "vv" */
 func_step_0:
				/* if there are no parameters, then skip this
				 * step, and go to step "1" */
				if (tv[tt - 1].expr == 0) {
					tv[tt - 1].step = 1;
					goto func_step_1;
				}
				/* otherwise compute next parameter */
				if (tv[tt - 1].expr->type == VLUE) {
					/* just put value on "vv" and continue
					 * in the same step "0" */
					if (vt == vs) {
						if (vs > SIZE_MAX / 2 / sizeof(struct vlue)) {
							fputs("Limit reached.\n", stderr);
							exit(EXIT_FAILURE);
						}
						vs *= 2;
						vv = GC_realloc(vv, vs * sizeof(struct vlue));
						if (vv == 0) {
							fputs("Out of memory.\n", stderr);
							exit(EXIT_FAILURE);
						}
					}
					vv[vt] = *tv[tt - 1].expr->data.vlue;
					vt++;
					tv[tt - 1].expr = tv[tt - 1].expr->next;
					goto func_step_0;
				}
				/* if call, then make new "todo", and restart */
				if (tt == ts) {
					if (ts > SIZE_MAX / 2 / sizeof(struct todo)) {
						fputs("Limit reached.\n", stderr);
						exit(EXIT_FAILURE);
					}
					ts *= 2;
					tv = GC_realloc(tv, ts * sizeof(struct todo));
					if (tv == 0) {
						fputs("Out of memory.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				tv[tt].func = tv[tt - 1].expr->data.call;
				tv[tt].step = 0;
				tv[tt].expr = tv[tt].func->next;
				tv[tt].parm = vt;
				tv[tt].symb = tv[tt - 1].symb;
				tv[tt].retv = vt;
				tv[tt].rets = st;
				tv[tt - 1].expr = tv[tt - 1].expr->next;
				tt++;
				goto start;
			} else if (tv[tt - 1].step == 1) {
				/* all parameters are stored on "vv" on
				 * positions "tv[tt - 1].parm" to "vt -1", so
				 * it is time to compute function body */
 func_step_1:
				f = tv[tt - 1].func->data.func;
				/* execute built-in function, save result in
				 * "vv[tv[tt - 1].retv", finish */
				if (f->type == CORE) {
					if (tv[tt - 1].retv == vs) {
						if (vs > SIZE_MAX / 2 / sizeof(struct vlue)) {
							fputs("Limit reached.\n", stderr);
							exit(EXIT_FAILURE);
						}
						vs *= 2;
						vv = GC_realloc(vv, vs * sizeof(struct vlue));
						if (vv == 0) {
							fputs("Out of memory.\n", stderr);
							exit(EXIT_FAILURE);
						}
					}
					f->body.core(vt - tv[tt - 1].parm, &vv[tv[tt - 1].parm]);
					vv[tv[tt - 1].retv] = vv[tv[tt - 1].parm];
					vt = tv[tt - 1].retv + 1;
					st = tv[tt - 1].rets;
					tt--;
					goto end;
				}
				/* check number of parameters */
				if (vt - tv[tt - 1].parm != f->body.user.parc) {
					fprintf(stderr, "Can not compute \"%s\" call. Wrong number of parameters.\n", f->name);
					exit(EXIT_FAILURE);
				}
				/* type of function is "USER"; if body is value
				 * then just copy it to "vv" stack and finish */
				if (f->body.user.expr->type == VLUE) {
					if (tv[tt - 1].retv == vs) {
						if (vs > SIZE_MAX / 2 / sizeof(struct vlue)) {
							fputs("Limit reached.\n", stderr);
							exit(EXIT_FAILURE);
						}
						vs *= 2;
						vv = GC_realloc(vv, vs * sizeof(struct vlue));
						if (vv == 0) {
							fputs("Out of memory.\n", stderr);
							exit(EXIT_FAILURE);
						}
					}
					vv[tv[tt - 1].retv] = *f->body.user.expr->data.vlue;
					vt = tv[tt - 1].retv + 1;
					st = tv[tt - 1].rets;
					tt--;
					goto end;
				}
				/* function body is a function call; define new
				 * symbols for all parameters, then evaluate
				 * body with new symbols defined; use the same
				 * todo "tv[tt - 1]" */
				for (i = 0; i < f->body.user.parc; i++) {
					if (st == ss) {
						if (ss > SIZE_MAX / 2 / sizeof(struct symb)) {
							fputs("Limit reached.\n", stderr);
							exit(EXIT_FAILURE);
						}
						ss *= 2;
						sv = GC_realloc(sv, ss * sizeof(struct symb));
						if (sv == 0) {
							fputs("Out of memory.\n", stderr);
							exit(EXIT_FAILURE);
						}
					}
					sv[st].name = f->body.user.parv[i];
					sv[st].vlue = tv[tt - 1].parm + i;
					if (i == 0) {
						/* first parameter is the last
						 * to look when searching */
						sv[st].next = SIZE_MAX;
					} else {
						/* next to look is previous */
						sv[st].next = st - 1;
					}
					st++;
				}
				tv[tt - 1].func = f->body.user.expr->data.call;
				tv[tt - 1].step = 0;
				tv[tt - 1].expr = tv[tt - 1].func->next;
				tv[tt - 1].parm = vt;
				tv[tt - 1].symb = f->body.user.parc == 0 ? SIZE_MAX : st - 1;
				/* retv, rets no change */
				goto end;
			}
			fputs("Wrong data.\n", stderr);
			exit(EXIT_FAILURE);
		}

		/* "tv[tt - 1].func->type" is not "FUNC", so this must be
		 * "VLUE", so this is holds name of function */
		a = tv[tt - 1].func->data.vlue->data.name;

		/* named function can be a symbol defined as a parameter of
		 * function or by "as" function call; search list of symbols;
		 * if match is found put value on "vv" stack and finish; search
		 * symbols before searching names of functions, because symbols
		 * should override names of functions */
		for (i = tv[tt - 1].symb; i != SIZE_MAX; i = sv[i].next)
			if (strcmp(a, sv[i].name) == 0) {
				if (tv[tt - 1].expr != 0) {
					fprintf(stderr, "Can not compute \"%s\" call. Wrong number of parameters.\n", a);
					exit(EXIT_FAILURE);
				}
				if (tv[tt - 1].retv == vs) {
					if (vs > SIZE_MAX / 2 / sizeof(struct vlue)) {
						fputs("Limit reached.\n", stderr);
						exit(EXIT_FAILURE);
					}
					vs *= 2;
					vv = GC_realloc(vv, vs * sizeof(struct vlue));
					if (vv == 0) {
						fputs("Out of memory.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				vv[tv[tt - 1].retv] = vv[sv[i].vlue];
				vt = tv[tt - 1].retv + 1;
				st = tv[tt - 1].rets;
				tt--;
				goto end;
			}

		/* function "if"; report error if there are no parameters; if
		 * there is only one parameter to compute, just compute it and
		 * finish; otherwise compute first parameter, check boolean
		 * result and decide to compute or skip next parameter */
		if (strcmp(a, "if") == 0) {
			if (tv[tt - 1].step == 0) {
 if_step_0:
				/* no parameters, error */
				if (tv[tt - 1].expr == 0) {
					fputs("Can not compute \"if\" call. Wrong number of parameters.\n", stderr);
					exit(EXIT_FAILURE);
				}
				/* one parameter only, immediately jump to step 
				 * "1" */
				if (tv[tt - 1].expr->next == 0) {
					tv[tt - 1].step = 1;
					goto if_step_1;
				}
				/* there are two or more parameters,
				 * immediately jump to step "2" */
				tv[tt - 1].step = 2;
				goto if_step_2;
			} else if (tv[tt - 1].step == 1) {
				/* compute given parameter, then finish; this
				 * parameter is "else" expression or expression 
				 * after true condition */
 if_step_1:
				if (tv[tt - 1].expr->type == VLUE) {
					/* value just put on "vv" and finish */
					if (tv[tt - 1].retv == vs) {
						if (vs > SIZE_MAX / 2 / sizeof(struct vlue)) {
							fputs("Limit reached.\n", stderr);
							exit(EXIT_FAILURE);
						}
						vs *= 2;
						vv = GC_realloc(vv, vs * sizeof(struct vlue));
						if (vv == 0) {
							fputs("Out of memory.\n", stderr);
							exit(EXIT_FAILURE);
						}
					}
					vv[tv[tt - 1].retv] = *tv[tt - 1].expr->data.vlue;
					vt = tv[tt - 1].retv + 1;
					st = tv[tt - 1].rets;
					tt--;
					goto end;
				}
				/* for call replace "todo" and restart */
				tv[tt - 1].func = tv[tt - 1].expr->data.call;
				tv[tt - 1].step = 0;
				tv[tt - 1].expr = tv[tt - 1].func->next;
				tv[tt - 1].parm = vt;
				/* symb, retv, rets no change */
				goto start;
			} else if (tv[tt - 1].step == 2) {
				/* compute boolean value, then continue in step
				 * "3" */
 if_step_2:
				if (tv[tt - 1].expr->type == VLUE) {
					/* value just put on "vv", set for next
					 * expression, go to step "3" */
					if (vt == vs) {
						if (vs > SIZE_MAX / 2 / sizeof(struct vlue)) {
							fputs("Limit reached.\n", stderr);
							exit(EXIT_FAILURE);
						}
						vs *= 2;
						vv = GC_realloc(vv, vs * sizeof(struct vlue));
						if (vv == 0) {
							fputs("Out of memory.\n", stderr);
							exit(EXIT_FAILURE);
						}
					}
					vv[vt] = *tv[tt - 1].expr->data.vlue;
					vt++;
					tv[tt - 1].step = 3;
					tv[tt - 1].expr = tv[tt - 1].expr->next;
					goto if_step_3;
				}
				/* for call make new "todo", set for next
				 * expression, later continue in step "3" */
				if (tt == ts) {
					if (ts > SIZE_MAX / 2 / sizeof(struct todo)) {
						fputs("Limit reached.\n", stderr);
						exit(EXIT_FAILURE);
					}
					ts *= 2;
					tv = GC_realloc(tv, ts * sizeof(struct todo));
					if (tv == 0) {
						fputs("Out of memory.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				tv[tt].func = tv[tt - 1].expr->data.call;
				tv[tt].step = 0;
				tv[tt].expr = tv[tt].func->next;
				tv[tt].parm = vt;
				tv[tt].symb = tv[tt - 1].symb;
				tv[tt].retv = vt;
				tv[tt].rets = st;
				tv[tt - 1].step = 3;
				tv[tt - 1].expr = tv[tt - 1].expr->next;
				tt++;
				goto start;
			} else if (tv[tt - 1].step == 3) {
				/* check boolean value; if true then compute
				 * next parameter in step "1"; otherwise skip
				 * parameter and go to step "0" */
 if_step_3:
				if (vv[vt - 1].type != BOOL) {
					fputs("Can not compute \"if\" call. Wrong type for condition.\n", stderr);
					exit(EXIT_FAILURE);
				}
				if (vv[vt - 1].data.bool) {
					/* true, compute expression and finish
					 * in step "1" */
					vt--;
					tv[tt - 1].step = 1;
					goto if_step_1;
				} else {
					/* false, skip parameter and restart
					 * from step "0" */
					vt--;
					tv[tt - 1].step = 0;
					tv[tt - 1].expr = tv[tt - 1].expr->next;
					goto if_step_0;
				}
			}
			fputs("Wrong data.\n", stderr);
			exit(EXIT_FAILURE);
		}

		/* function "as"; report error if there are no parameters; if
		 * there is only one parameter to compute, just compute it and
		 * finish; otherwise compute name and value, define symbol and
		 * start from the beginning */
		if (strcmp(a, "as") == 0) {
			if (tv[tt - 1].step == 0) {
 as_step_0:
				/* if there are no parameters to compute report
				 * error */
				if (tv[tt - 1].expr == 0) {
					fputs("Can not compute \"as\" call. Wrong number of parameters.\n", stderr);
					exit(EXIT_FAILURE);
				}
				/* if there is only one parameter to compute,
				 * then go immediately to step "1" */
				if (tv[tt - 1].expr->next == 0) {
					tv[tt - 1].step = 1;
					goto as_step_1;
				}
				/* there are two or more parameters, then
				 * immediately go to step "2" */
				tv[tt - 1].step = 2;
				goto as_step_2;
			} else if (tv[tt - 1].step == 1) {
				/* compute given parameter and finish; this is
				 * the last expression in "as" call */
 as_step_1:
				if (tv[tt - 1].expr->type == VLUE) {
					/* just put on "vv" and finish */
					if (tv[tt - 1].retv == vs) {
						if (vs > SIZE_MAX / 2 / sizeof(struct vlue)) {
							fputs("Limit reached.\n", stderr);
							exit(EXIT_FAILURE);
						}
						vs *= 2;
						vv = GC_realloc(vv, vs * sizeof(struct vlue));
						if (vv == 0) {
							fputs("Out of memory.\n", stderr);
							exit(EXIT_FAILURE);
						}
					}
					vv[tv[tt - 1].retv] = *tv[tt - 1].expr->data.vlue;
					vt = tv[tt - 1].retv + 1;
					st = tv[tt - 1].rets;
					tt--;
					goto end;
				}
				/* for call change current todo "tv[tt - 1]"
				 * and restart */
				tv[tt - 1].func = tv[tt - 1].expr->data.call;
				tv[tt - 1].step = 0;
				tv[tt - 1].expr = tv[tt - 1].func->next;
				tv[tt - 1].parm = vt;
				tv[tt - 1].symb = tv[tt - 1].rets == st ? SIZE_MAX : st - 1;
				/* retv, rets no change */
				goto start;
			} else if (tv[tt - 1].step == 2) {
				/* compute name */
 as_step_2:
				if (tv[tt - 1].expr->type == VLUE) {
					/* value just put on "vv", set for next
					 * expression, continue in step "3" */
					if (vt == vs) {
						if (vs > SIZE_MAX / 2 / sizeof(struct vlue)) {
							fputs("Limit reached.\n", stderr);
							exit(EXIT_FAILURE);
						}
						vs *= 2;
						vv = GC_realloc(vv, vs * sizeof(struct vlue));
						if (vv == 0) {
							fputs("Out of memory.\n", stderr);
							exit(EXIT_FAILURE);
						}
					}
					vv[vt] = *tv[tt - 1].expr->data.vlue;
					vt++;
					tv[tt - 1].step = 3;
					tv[tt - 1].expr = tv[tt - 1].expr->next;
					goto as_step_3;
				}
				/* for call make new "todo", later continue in
				 * step "3" */
				if (tt == ts) {
					if (ts > SIZE_MAX / 2 / sizeof(struct todo)) {
						fputs("Limit reached.\n", stderr);
						exit(EXIT_FAILURE);
					}
					ts *= 2;
					tv = GC_realloc(tv, ts * sizeof(struct todo));
					if (tv == 0) {
						fputs("Out of memory.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				tv[tt].func = tv[tt - 1].expr->data.call;
				tv[tt].step = 0;
				tv[tt].expr = tv[tt].func->next;
				tv[tt].parm = vt;
				tv[tt].symb = tv[tt - 1].symb;
				tv[tt].retv = vt;
				tv[tt].rets = st;
				tv[tt - 1].step = 3;
				tv[tt - 1].expr = tv[tt - 1].expr->next;
				tt++;
				goto start;
			} else if (tv[tt - 1].step == 3) {
				/* check name, compute value */
 as_step_3:
				if (vv[vt - 1].type != NAME) {
					fputs("Can not compute \"as\" call. Wrong type for name of symbol.\n", stderr);
					exit(EXIT_FAILURE);
				}
				if (tv[tt - 1].expr->type == VLUE) {
					/* value just put on "vv", set for next
					 * expression, continue in step "4" */
					if (vt == vs) {
						if (vs > SIZE_MAX / 2 / sizeof(struct vlue)) {
							fputs("Limit reached.\n", stderr);
							exit(EXIT_FAILURE);
						}
						vs *= 2;
						vv = GC_realloc(vv, vs * sizeof(struct vlue));
						if (vv == 0) {
							fputs("Out of memory.\n", stderr);
							exit(EXIT_FAILURE);
						}
					}
					vv[vt] = *tv[tt - 1].expr->data.vlue;
					vt++;
					tv[tt - 1].step = 4;
					tv[tt - 1].expr = tv[tt - 1].expr->next;
					goto as_step_4;
				}
				/* for call make new "todo", later continue in
				 * step "4" */
				if (tt == ts) {
					if (ts > SIZE_MAX / 2 / sizeof(struct todo)) {
						fputs("Limit reached.\n", stderr);
						exit(EXIT_FAILURE);
					}
					ts *= 2;
					tv = GC_realloc(tv, ts * sizeof(struct todo));
					if (tv == 0) {
						fputs("Out of memory.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				tv[tt].func = tv[tt - 1].expr->data.call;
				tv[tt].step = 0;
				tv[tt].expr = tv[tt].func->next;
				tv[tt].parm = vt;
				tv[tt].symb = tv[tt - 1].symb;
				tv[tt].retv = vt;
				tv[tt].rets = st;
				tv[tt - 1].step = 4;
				tv[tt - 1].expr = tv[tt - 1].expr->next;
				tt++;
				goto start;
			} else if (tv[tt - 1].step == 4) {
				/* define symbol and restart from step "0";
				 * "vv[vt - 2]" contains name for symbol, and
				 * "vv[vt - 1]" contains value for symbol */
 as_step_4:
				if (st == ss) {
					if (ss > SIZE_MAX / 2 / sizeof(struct symb)) {
						fputs("Limit reached.\n", stderr);
						exit(EXIT_FAILURE);
					}
					ss *= 2;
					sv = GC_realloc(sv, ss * sizeof(struct symb));
					if (sv == 0) {
						fputs("Out of memory.\n", stderr);
						exit(EXIT_FAILURE);
					}
				}
				sv[st].name = vv[vt - 2].data.name;
				sv[st].vlue = vt - 1;
				if (tv[tt - 1].rets == st) {
					/* first symbol of "as" */
					sv[st].next = tv[tt - 1].symb;
				} else {
					/* more symbols, continued */
					sv[st].next = st - 1;
				}
				st++;
				tv[tt - 1].step = 0;
				goto as_step_0;
			}
			fputs("Wrong data.\n", stderr);
			exit(EXIT_FAILURE);
		}

		/* find function definition, update calling expression */
		n = tree_search(d, a);
		if (n == 0) {
			fprintf(stderr, "Can not compute function call. Function \"%s\" not defined.\n", a);
			exit(EXIT_FAILURE);
		}
		tv[tt - 1].func->type = FUNC;
		tv[tt - 1].func->data.func = (struct func *)n->vlue;
		goto start;
 end:
		;
	}

	return vv;
}
