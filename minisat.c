#include SWI-Prolog.h
#include stdio.h
#include stdlib.h
#include string.h
#include ctype.h
#include "minisat.h"

static int get_one_literal(term_t, int*);
static wrap_solver _slv_;                   /* Solver */

static foreign_t
sat_solver_new()
{
    _slv_ = wrap_solver_new();
    PL_succeed;
}

static foreign_t
sat_new_var(term_t var)
{
    int v;
   
    v = wrap_solver_new_var(_slv_);
   
    return(PL_unify_integer(var, v));
}

#if 0                                       /* 互換性のため */
static foreign_t
sat_pos_var(term_t var, term_t lit)
{
    int v;
    int l;
   
    PL_get_integer(var, &v);
    l = wrap_lit_pos_var(v);
   
    return(PL_unify_integer(lit, l));
}

static foreign_t
sat_neg_var(term_t var, term_t lit)
{
    int v;
    int l;
   
    PL_get_integer(var, &v);
    l = wrap_lit_neg_var(v);
   
    return(PL_unify_integer(lit, l));
}
#endif

#define CLS_SIZE 100

static foreign_t
sat_add_clause(term_t cls)
{
    term_t head = PL_new_term_ref();        /* variable for the elements */
    term_t list = PL_copy_term_ref(cls);    /* copy as we need to write */
   
    int v;                                  /* SAT 変数 */
    int i = 0;
    int lts[CLS_SIZE];
   
    while (PL_get_list(list, head, list)) {
        if (get_one_literal(head, &v)) {
            lts[i++] = v;
        } else {
            PL_fail;
        }
       
        if (i = CLS_SIZE) {
            printf("MINISAT: a clause is too long.\n");
            PL_fail;
        }
    }
    if (i  0) {
        wrap_solver_add_clause(_slv_, lts, i);
        PL_succeed;
    }  else {
        printf("MINISAT: invalid clause (be a list).\n");
        PL_fail;
    }
}

static int
get_one_literal(term_t t_tmp, int *l)
{
    atom_t t_func;
    term_t t_term = PL_copy_term_ref(t_tmp); /* copy as we need to write */
    int    t_arity;
   
    int  v;                                 /* SAT 変数 */
    const char *f;                          /* fanctor (+ / -) */
    int  i = 0;                             /* 「-」の数 */
   
    do {
        /* -(X), +(X)を再帰的にとりだす。 */
        if (PL_get_name_arity(t_term, &t_func, &t_arity)) {
            f = PL_atom_chars(t_func);
           
            if (t_arity == 1) {
                if (strncmp(f, "-", 1) == 0) {
                    i ++;
                } else if (strncmp(f, "+", 1) == 0) {
                    ;
                } else {
                    printf("MINISAT: invalid functor %s/%d.\n", f, t_arity);
                    return(0);
                }
            } else {
                    printf("MINISAT: invalid functor %s/%d.\n", f, t_arity);
                    return(0);
            }
        }
        /* -(X), +(X)を再帰的にとりだす。 */
    } while (PL_get_arg(1, t_term, t_term));
   
    if (PL_get_integer(t_term, &v)) {
        if (i % 2 == 0) {                   /* 偶数回？ */
            *l = wrap_lit_pos_var(v);
        } else {                            /* 奇数回である。 */
            *l = wrap_lit_neg_var(v);       
        }
    } else {
        return(0);
    }
}

static foreign_t
sat_solve()
{
    int dummy;
   
    if (wrap_solver_solve(_slv_, &dummy, 0)) {
        PL_succeed;
    } else {
        PL_fail;
    }
}

static foreign_t
sat_ref_var(term_t var)
{
    int v;
   
    if (PL_is_integer(var)) {
        PL_get_integer(var, &v);
   
        switch (wrap_solver_ref_var(_slv_, v)) {
        case 1:
            PL_succeed;
            break;
        case 0:
            PL_fail;
            break;
        default:
            printf("MINISAT: invalid variable.\n");
            PL_fail;
            break;
        }
    } else {
        printf("MINISAT: invalid variable.\n");
        PL_fail;
    }
}

static foreign_t
sat_solver_free()
{
    wrap_solver_free(_slv_);
    PL_succeed;
}

install_t
install_minisat()
{
    PL_register_foreign("sat_solver_new",  0, sat_solver_new,  0);
    PL_register_foreign("sat_new_var",     1, sat_new_var,     0);
#if 0                                       /* 互換性のため */
    PL_register_foreign("sat_pos_var",     2, sat_pos_var,     0);
    PL_register_foreign("sat_neg_var",     2, sat_neg_var,     0);
#endif
    PL_register_foreign("sat_add_clause",  1, sat_add_clause,  0);
    PL_register_foreign("sat_solve",       0, sat_solve,       0);
    PL_register_foreign("sat_ref_var",     1, sat_ref_var,     0);
    PL_register_foreign("sat_solver_free", 0, sat_solver_free, 0);
   
    printf("\nminisat is installed.\n\n");
}

/* END */

/*
  % g++ -I. -I/usr/local/include -fpic -c Solver.C
  % g++ -I. -I/usr/local/include -fpic -c minisat-wrap.cpp
  % gcc -I. -I/usr/local/include -I/usr/local/lib/pl-5.6.64/include -fpic -c minisat.c
  % g++ -shared -o minisat.so Solver.o minisat-wrap.o minisat.o -L/usr/local/lib/
  % pl
  ?- load_foreign_library(minisat).
  go :-
        sat_solver_new,
        sat_new_var(A),
        sat_new_var(B),
        sat_add_clause([A, B]),
        sat_add_clause([-A, B]),
        sat_solve, !,
        (sat_ref_var(A) -
         write('A is true'); write('A is false')), nl,
        (sat_ref_var(B) -
         write('B is true'); write('B is false')), nl,
        sat_solver_free.
*/
