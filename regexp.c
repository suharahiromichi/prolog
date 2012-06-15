#include SWI-Prolog.h 
#include stdio.h 
#include stdlib.h 
#include string.h 
#include ctype.h 
#include "oniguruma.h" 

#define NAMES 100 
#define NAME_LEN 1024 

static int regexp_main(char*, char*); 
static int name_callback(const UChar*, const UChar*, int, int*, regex_t*, void*); 

static foreign_t 
re_search(term_t reg, term_t str) 
{ 
    char *a; 
    char *b; 

    if (PL_get_chars(reg, &a, CVT_ALL)) { 
        if (PL_get_chars(str, &b, CVT_ALL)) { 
            if (regexp_main(a,              /* "(?fooa*)(?barb*)(?fooc*)" */ 
                            b)              /* "aaaaaaabbbbbbbbccc" */ 
                == 0) { 
                PL_succeed; 
            } 
        } 
    } 
    PL_fail; 
} 

static char data_str[NAME_LEN]; 
static char data_value[NAME_LEN]; 

static struct { 
    char n[NAME_LEN]; 
    int  b; 
    int  e; 
} data_rslt[NAMES]; 

static foreign_t 
re_name(term_t name, term_t value) 
{ 
    char *a; 
    int len; 
     
    int i; 
     
    if (PL_get_chars(name, &a, CVT_ALL)) { 
        for (i = 1; i  NAMES; i ++) { 
            if (data_rslt[i].n[0] == 0) { 
                PL_fail; 
            } 
             
            if (! strcmp(a, data_rslt[i].n)) { 
                memset(data_value, 0, NAMES); 
                strncpy(data_value, 
                        data_str + data_rslt[i].b, 
                        data_rslt[i].e - data_rslt[i].b); 
                PL_unify_string_chars(value, data_value); 
                PL_succeed; 
            } 
        } 
    } 
    PL_fail; 
} 

static int 
regexp_main(char *pat0, char *str0) 
{ 
    int r; 
    unsigned char *start, *range, *end; 
    regex_t* reg; 
    OnigErrorInfo einfo; 
    OnigRegion *region; 
     
    static UChar* pattern; 
    static UChar* str; 
    int i; 
     
    pattern = (UChar* )pat0; 
    str = (UChar* )str0; 
     
    strcpy(data_str, str); 
    for (i = 0; i  NAMES; i ++) { 
        data_rslt[i].n[0] = 0; 
    } 
     
    r = onig_new(&reg, pattern, pattern + strlen((char* )pattern), 
                 ONIG_OPTION_DEFAULT, ONIG_ENCODING_ASCII, ONIG_SYNTAX_DEFAULT, &einfo); 
    if (r != ONIG_NORMAL) { 
        char s[ONIG_MAX_ERROR_MESSAGE_LEN]; 
        onig_error_code_to_str(s, r, &einfo); 
        return -1; 
    } 
     
    region = onig_region_new(); 
     
    end   = str + strlen((char* )str); 
    start = str; 
    range = end; 
    r = onig_search(reg, str, end, start, range, region, ONIG_OPTION_NONE); 
    if (r = 0) { 
        r = onig_foreach_name(reg, name_callback, (void* )region); 
    } 
    else if (r == ONIG_MISMATCH) { 
        return 1; 
    } 
    else {                                  /* error */ 
        char s[ONIG_MAX_ERROR_MESSAGE_LEN]; 
        onig_error_code_to_str(s, r); 
        return -1; 
    } 
     
    onig_region_free(region, 1); 
    /* 1:free self, 0:free contents only */ 
    onig_free(reg); 
    onig_end(); 
    return 0; 
} 

static int 
name_callback(const UChar* name, const UChar* name_end, 
     int ngroup_num, int* group_nums, 
     regex_t* reg, void* arg) 
{ 
    int i, gn, ref; 
    char* s; 
    OnigRegion *region = (OnigRegion* )arg; 
     
    for (i = 0; i  ngroup_num; i++) { 
        gn = group_nums[i]; 
        ref = onig_name_to_backref_number(reg, name, name_end, region); 
        s = (ref == gn ? "*" : ""); 
         
        strcpy(data_rslt[gn].n, name); 
        data_rslt[gn].b = region-beg[gn]; 
        data_rslt[gn].e = region-end[gn]; 
    } 
    return 0;                               /* 0: continue */ 
} 


install_t 
install_regexp() 
{ 
    PL_register_foreign("re_search", 2, re_search, 0); 
    PL_register_foreign("re_name",   2, re_name,   0); 
    printf("regexp is instelled\n"); 
} 

/* END */ 

/* 
  % gcc -I/usr/local/lib/pl-5.6.64/include -fpic -c regexp.c 
  % gcc -shared -o regexp.so regexp.o 
  % pl 
  ?- load_foreign_library(regexp). 
  ?-  regexp_name("bbb", X), 
  X = "222". 
  true. 
   
  (atom) 
  ?-  regexp_name('bbb', X), 
  X = '222'. 
  true. 
   
*/
