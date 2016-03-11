/* 
 * Copyright 1998 Michael H. Buselli <cosine@cosine.org> Copyright 2000-2002
 * Wessel Dankers <wsl@nl.linux.org> Copyright 2010-2011 Leszek Dubiel
 * <leszek@dubiel.pl>
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
 * 
 * Original code for avl trees by Michael H. Buselli <cosine@cosine.org>.
 * 
 * Modified by Wessel Dankers <wsl@nl.linux.org> to add a bunch of bloat to the
 * sourcecode, change the interface and squash a few bugs.
 * 
 * Finally simplified by Leszek Dubiel <leszek@dubiel.pl>, to store data in a
 * form of name-value pairs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gc/gc.h>

#include "tree.h"

#define NODE_DPTH(n) ((n) != 0 ? (n)->dpth : 0)
#define LEFT_DPTH(n) (NODE_DPTH((n)->left))
#define RGHT_DPTH(n) (NODE_DPTH((n)->rght))
#define CALC_DPTH(n) ((LEFT_DPTH(n) > RGHT_DPTH(n) ? LEFT_DPTH(n) : RGHT_DPTH(n)) + 1)

void tree_balance(struct tree *t, struct node *n)
{
	struct node *c, *g, *p, **s;	/* child, grand child, parent, slot */

	if (t == 0 || n == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}

	while (n != 0) {

		/* p is parent; s is a place where n is saved */
		p = n->root;
		if (p == 0)
			s = &t->root;
		else
			s = n == p->left ? &p->left : &p->rght;

		/* rebalancing; for cases see wikipedia */
		if (RGHT_DPTH(n) - LEFT_DPTH(n) < -1) {
			c = n->left;
			if (LEFT_DPTH(c) >= RGHT_DPTH(c)) {
				/* left left case */
				n->left = c->rght;
				if (n->left != 0)
					n->left->root = n;
				c->rght = n;
				n->root = c;
				*s = c;
				c->root = p;
				n->dpth = CALC_DPTH(n);
				c->dpth = CALC_DPTH(c);
			} else {
				/* left right case */
				g = c->rght;
				n->left = g->rght;
				if (n->left != 0)
					n->left->root = n;
				c->rght = g->left;
				if (c->rght != 0)
					c->rght->root = c;
				g->rght = n;
				if (g->rght != 0)
					g->rght->root = g;
				g->left = c;
				if (g->left != 0)
					g->left->root = g;
				*s = g;
				g->root = p;
				n->dpth = CALC_DPTH(n);
				c->dpth = CALC_DPTH(c);
				g->dpth = CALC_DPTH(g);
			}
		} else if (RGHT_DPTH(n) - LEFT_DPTH(n) > 1) {
			c = n->rght;
			if (RGHT_DPTH(c) >= LEFT_DPTH(c)) {
				/* right right case */
				n->rght = c->left;
				if (n->rght != 0)
					n->rght->root = n;
				c->left = n;
				n->root = c;
				*s = c;
				c->root = p;
				n->dpth = CALC_DPTH(n);
				c->dpth = CALC_DPTH(c);
			} else {
				/* right left case */
				g = c->left;
				n->rght = g->left;
				if (n->rght != 0)
					n->rght->root = n;
				c->left = g->rght;
				if (c->left != 0)
					c->left->root = c;
				g->left = n;
				if (g->left != 0)
					g->left->root = g;
				g->rght = c;
				if (g->rght != 0)
					g->rght->root = g;
				*s = g;
				g->root = p;
				n->dpth = CALC_DPTH(n);
				c->dpth = CALC_DPTH(c);
				g->dpth = CALC_DPTH(g);
			}
		} else {
			n->dpth = CALC_DPTH(n);
		}

		n = p;
	}
}

struct node *tree_search(struct tree *t, char *a)
{
	struct node *n;
	int c;

	if (t == 0 || a == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}

	n = t->root;
	while (n != 0) {
		c = strcmp(a, n->name);
		if (c < 0)
			n = n->left;
		else if (c > 0)
			n = n->rght;
		else
			return n;
	}

	return 0;
}

struct node *tree_insert(struct tree *t, char *a)
{
	struct node *n, *m;
	int c;

	if (t == 0 || a == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}

	m = GC_malloc(sizeof(struct node));
	if (m == 0) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	m->name = a;
	m->vlue = 0;
	m->tree = t;
	m->left = m->rght = 0;
	m->dpth = 1;

	if (t->root == 0) {
		m->prev = m->next = m->root = 0;
		t->frst = t->last = t->root = m;
		return m;
	}

	n = t->root;
	for (;;) {
		c = strcmp(m->name, n->name);
		if (c < 0) {
			if (n->left != 0) {
				n = n->left;
			} else {
				m->root = n;
				if (n->prev != 0) {
					m->prev = n->prev;
					n->prev->next = m;
				} else {
					m->prev = 0;
					t->frst = m;
				}
				m->next = n;
				n->prev = m;
				n->left = m;
				break;
			}
		} else if (c > 0) {
			if (n->rght != 0) {
				n = n->rght;
			} else {
				m->root = n;
				m->prev = n;
				if (n->next != 0) {
					m->next = n->next;
					n->next->prev = m;
				} else {
					m->next = 0;
					t->last = m;
				}
				n->next = m;
				n->rght = m;
				break;
			}
		} else {
			return 0;
		}
	}
	tree_balance(t, n);

	return m;
}

struct node *tree_delete(struct tree *t, char *a)
{
	struct node *n, *m, **s;
	int c;

	if (t == 0 || a == 0) {
		fputs("Null pointer.\n", stderr);
		exit(EXIT_FAILURE);
	}

	/* locate node m with name a */
	m = t->root;
	while (m != 0) {
		c = strcmp(a, m->name);
		if (c < 0)
			m = m->left;
		else if (c > 0)
			m = m->rght;
		else
			break;
	}
	if (m == 0)
		return 0;

	/* set s to point to place where m is saved */
	if (m->root == 0)
		s = &t->root;
	else
		s = m == m->root->left ? &m->root->left : &m->root->rght;

	/* remove node m from list next, prev */
	if (m->prev != 0)
		m->prev->next = m->next;
	else
		t->frst = m->next;
	if (m->next != 0)
		m->next->prev = m->prev;
	else
		t->last = m->prev;

	/* remove node from tree */
	if (m->left == 0) {
		*s = m->rght;
		if (m->rght != 0)
			m->rght->root = m->root;
		if (m->root != 0)
			tree_balance(t, m->root);
	} else if (m->rght == 0) {
		*s = m->left;
		m->left->root = m->root;
		if (m->root != 0)
			tree_balance(t, m->root);
	} else {
		/* replace node with righmost node of left subtree */
		*s = m->prev;
		if (m->prev == m->left) {
			n = m->prev;
		} else {
			n = m->prev->root;
			n->rght = m->prev->left;
			if (n->rght != 0)
				n->rght->root = n;
			m->prev->left = m->left;
			m->left->root = m->prev;
		}
		m->prev->rght = m->rght;
		m->prev->root = m->root;
		m->rght->root = m->prev;
		tree_balance(t, n);
	}

	return m;
}
