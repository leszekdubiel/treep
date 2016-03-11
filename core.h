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

void core_do(size_t, struct vlue *);
void core_is(size_t, struct vlue *);

void core_add(size_t, struct vlue *);
void core_sub(size_t, struct vlue *);
void core_mul(size_t, struct vlue *);
void core_div(size_t, struct vlue *);
void core_int(size_t, struct vlue *);
void core_flo(size_t, struct vlue *);
void core_cil(size_t, struct vlue *);
void core_min(size_t, struct vlue *);
void core_max(size_t, struct vlue *);
void core_ops(size_t, struct vlue *);
void core_abs(size_t, struct vlue *);
void core_rnd(size_t, struct vlue *);

void core_zer(size_t, struct vlue *);
void core_pos(size_t, struct vlue *);
void core_neg(size_t, struct vlue *);
void core_equ(size_t, struct vlue *);
void core_gre(size_t, struct vlue *);
void core_les(size_t, struct vlue *);

void core_yes(size_t, struct vlue *);
void core_no(size_t, struct vlue *);
void core_not(size_t, struct vlue *);
void core_and(size_t, struct vlue *);
void core_or(size_t, struct vlue *);

void core_numb(size_t, struct vlue *);
void core_name(size_t, struct vlue *);
void core_text(size_t, struct vlue *);

void core_same(size_t, struct vlue *);
void core_bfor(size_t, struct vlue *);
void core_aftr(size_t, struct vlue *);
void core_leng(size_t, struct vlue *);
void core_escp(size_t, struct vlue *);
void core_unes(size_t, struct vlue *);
void core_uppr(size_t, struct vlue *);
void core_lowr(size_t, struct vlue *);
void core_head(size_t, struct vlue *);
void core_tail(size_t, struct vlue *);
void core_join(size_t, struct vlue *);
void core_part(size_t, struct vlue *);
void core_indx(size_t, struct vlue *);
void core_ptrn(size_t, struct vlue *);
void core_mtch(size_t, struct vlue *);
void core_msta(size_t, struct vlue *);
void core_mend(size_t, struct vlue *);

void core_new(size_t, struct vlue *);
void core_ref(size_t, struct vlue *);
void core_any(size_t, struct vlue *);
void core_qty(size_t, struct vlue *);
void core_def(size_t, struct vlue *);
void core_ins(size_t, struct vlue *);
void core_del(size_t, struct vlue *);
void core_set(size_t, struct vlue *);
void core_get(size_t, struct vlue *);

void core_root(size_t, struct vlue *);
void core_frst(size_t, struct vlue *);
void core_last(size_t, struct vlue *);
void core_left(size_t, struct vlue *);
void core_rght(size_t, struct vlue *);
void core_next(size_t, struct vlue *);
void core_prev(size_t, struct vlue *);
void core_vlue(size_t, struct vlue *);
void core_tree(size_t, struct vlue *);
void core_node(size_t, struct vlue *);
void core_main(size_t, struct vlue *);

void core_read(size_t, struct vlue *);
void core_back(size_t, struct vlue *);
void core_wrte(size_t, struct vlue *);
void core_dump(size_t, struct vlue *);

void core_void(size_t, struct vlue *);
void core_spac(size_t, struct vlue *);
void core_tabu(size_t, struct vlue *);
void core_line(size_t, struct vlue *);
void core_lpar(size_t, struct vlue *);
void core_rpar(size_t, struct vlue *);

void core_time(size_t, struct vlue *);
void core_repr(size_t, struct vlue *);
void core_pars(size_t, struct vlue *);
void core_iden(size_t, struct vlue *);

void core_vers(size_t, struct vlue *);
void core_info(size_t, struct vlue *);
void core_exit(size_t, struct vlue *);
