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

struct node {
	char *name;
	void *vlue;
	struct tree *tree;
	struct node *root;
	struct node *next, *prev;
	struct node *left, *rght;
	unsigned char dpth;
};

struct tree {
	struct node *root;
	struct node *frst, *last;
};

struct node *tree_search(struct tree *, char *);
struct node *tree_insert(struct tree *, char *);
struct node *tree_delete(struct tree *, char *);
