#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "strarr.h"
#include "strarr_iter.h"
#include "shelltree.h"

enum
{
    BUF_SIZE = 1024, /* for variables */
};

/* Key characters */
const char QUOT[] = "\'\"";
const char SPACES[] = " \t\n";
const char UNARY[] = ";<>&|";
const char DOUBLE[] = ">&|";
const char L_BRACKET = '(';
const char R_BRACKET = ')';
const char VAR = '$';
const char SLASH = '\\';
const char COMMENT = '#';

/* End tokens */
strarr ENDS_PIPE   = (char *[]){ "&&", "||", NULL }; /* After pipe */
strarr ENDS_NEXTIF = (char *[]){ NULL }; /* After next-if */
strarr ENDS_DFLT   = (char *[]){ NULL }; /* Default */

extern char BASH_NAME[];

/* Supported variables */
const char * const VARS[] = { "HOME", "SHELL", "USER", "EUID", NULL };

/* The function replaces a part of '*pstr' from 'pos' of length 'len_old' by 'frag' */
void _str_repl(char **pstr, int pos, int len_old, char *frag);

/* The function returns copy of sequence after $ */
char * _get_var(char *str, int pos);

/* The function returns a value of supported variable */
char * _get_var_val(char *var);

/* The function replaces all variables in string */
void _repl_var(char **pstr, strarr vars);

/* The function replaces escape sequences in string;
 * if 'seq' is NULL, then processes any character;
 * otherwise processes only characters from 'seq' */
void _repl_escape(char **pstr, const char *seq);

/* The function returns a fragment ['begin', 'end') of 'arr' */
char * _cut(strarr arr, const sait begin, const sait end);

/* The function adds a fragment ['begin', 'end') of 'src' to '*dst';
 * if to_repl_esc set on 1, then replaces escape sequences */
void _strarr_addn(strarr *dst, strarr src, const sait begin, const sait end, int to_repl_esc, const char *seq);

/* The function returns a category of a string for _check_syntax function */
int _ctg(const char *str);

/* The function checks if the syntax is correct;
 * if syntax is correct, returns 0;
 * otherwise returns error code */
int _check_syntax(strarr arr);

/* The function creates and returns ShTree by one part of parsed strarr */
ShTree * _st_create_one_tree(strarr arr, int *pos, strarr ends);

/* The function creates and returns ShTree of one command sequence (until ; or &) by part of parsed strarr */
ShTree * _st_create_sub_and_next(strarr arr, int *pos);

void
_str_repl(char **pstr, int pos, int len_old, char *frag)
{
    const int len_s = strlen(*pstr);
    const int len_new = strlen(frag);
    char *res = calloc(len_s - len_old + len_new + 1, sizeof(*res));
    strncpy(res, *pstr, pos);
    strcpy(res + pos, frag);
    strcpy(res + pos + len_new, *pstr + pos + len_old);
    free(*pstr);
    *pstr = res;
}

char *
_get_var(char *str, int pos)
{
    register int i = pos;
    while (i < strlen(str) && (isalnum(str[i]) || str[i] == '_')) {
        ++i;
    }
    char *res = calloc(i - pos + 1, sizeof(*res));
    strncpy(res, str + pos, i - pos);
    return res;
}

char *
_get_var_val(char *var)
{
    if (strcmp(var, "EUID") == 0) {
        char *res = calloc(BUF_SIZE, sizeof(*res));
        sprintf(res, "%d", getuid());
        return res;
    }
    char *tmp = getenv(var);
    char *res = calloc(strlen(tmp) + 1, sizeof(*res));
    strcpy(res, tmp);
    return res;
}

void
_repl_var(char **pstr, strarr vars)
{
    int i = 0;
    while (i < strlen(*pstr)) { /* Length of '*pstr' may change */
        char c = (*pstr)[i];
        if (c == '\\') {
            i += 2;
        } else if (c == '$') {
            char *var = _get_var(*pstr, i + 1);
            if (strlen(var) == 0) {
                ++i;
            } else {
                int p = strarr_find(vars, var);
                if (p == -1) {
                    _str_repl(pstr, i, strlen(var) + 1, "");
                } else {
                    char *var_val = _get_var_val(var);
                    _str_repl(pstr, i, strlen(var) + 1, var_val);
                    i += strlen(var_val);
                    free(var_val);
                }
            }
            free(var);
        } else {
            ++i;
        }
    }
}

void
_repl_escape(char **pstr, const char *seq)
{
    const int len = strlen(*pstr);
    
    /* Allocates tmp string (it may require less memory than allocated) */
    char *tmp = calloc(len + 1, sizeof(*tmp));

    /* Modifies string */
    register int i = 0; /* Position in '*pstr' array */
    register int j = 0; /* Position in 'tmp' array */
    while (i < len - 1) {
        if ((*pstr)[i] == SLASH && (seq == NULL || strchr(seq, (*pstr)[i + 1]) != NULL)) {
            /* If finds escape sequence, replaces it */
            tmp[j++] = (*pstr)[i + 1];
            i += 2;
        } else {
            /* Otherwise copies character */
            tmp[j++] = (*pstr)[i++];
        }
    }
    if (i == len - 1) {
        tmp[j] = (*pstr)[i];
    }

    /* If more memory has been allocated than needed, then it is freed */
    char *res = calloc(strlen(tmp) + 1, sizeof(*res));
    strcpy(res, tmp);
    free(tmp);

    free(*pstr);
    *pstr = res;
}

char *
_cut(strarr arr, const sait begin, const sait end)
{
    /* Calculates result size */
    register int size = 0;
    if (begin[0] == end[0]) {
        size += end[1] - begin[1];
    } else {
        size += strlen(arr[begin[0]]) - begin[1];
        for (int i = begin[0] + 1; i < end[0]; ++i) {
            size += strlen(arr[i]);
        }
        size += end[1];
    }

    /* Allocates memory for result string */
    char *res = calloc(size + 1, sizeof(*res));
    /* Fills result string */
    if (begin[0] == end[0]) {
        strncpy(res, arr[begin[0]] + begin[1], end[1] - begin[1]);
    } else {
        register int count = 0;
        strncpy(res + count, arr[begin[0]] + begin[1], strlen(arr[begin[0]]) - begin[1]);
        count += strlen(arr[begin[0]]) - begin[1];
        for (int i = begin[0] + 1; i < end[0]; ++i) {
            strncpy(res + count, arr[i], strlen(arr[i]));
            count += strlen(arr[i]);
        }
        strncpy(res + count, arr[end[0]], end[1]);
    }

    return res;
}

void
_strarr_addn(strarr *dst, strarr src, const sait begin, const sait end, int to_repl_esc, const char *seq)
{
    /* Gets string */
    char *tmp = _cut(src, begin, end);
    /* Replaces variables */
    _repl_var(&tmp, (strarr)VARS);
    /* Replaces escape sequences if it necessary */
    if (to_repl_esc) {
        _repl_escape(&tmp, seq);
    }
    /* Adds string to array */
    strarr_add(dst, tmp);
    /* Frees memory */
    free(tmp);
}

strarr
parse(strarr inarr)
{
    strarr outarr = strarr_init();

    const int arr_size = strarr_len(inarr);
    char quot = 0; /* Flag of quot marks: possible values: \0 or \" or \' */
    const sait end = { arr_size, 0 };
    sait begin = { 0, 0 };
    sait i = { 0, 0 };
    sait tmp = { 0, 0 };
    /* Iterates by characters */
    while (sait_cmp(i, end)) {
        char c = sait_ccur(inarr, i); /* Current character */
        if (quot) { /* If inside quots */
            if (c == quot) {
                /* Adds the current word to array */
                const char seq[] = { SLASH, VAR, quot, 0 };
                _strarr_addn(&outarr, inarr, begin, i, 1, seq);
                /* Sets quot flag on 0 */
                quot = 0;
                /* Moves to the next word (moves to the next character and set begin marker on it) */
                sait_incr(inarr, i, 1);
                sait_asgn(begin, i);
            } else if (c == SLASH) {
                /* Moves to the next-next character */
                if (sait_rpos(inarr, i) != 1) {
                    sait_incr(inarr, i, 2);
                } else {
                    sait_incr(inarr, i, 1);
                }
            } else {
                /* Moves to the next character */
                sait_incr(inarr, i, 1);
            }
        } else { /* If outside quots */
            if (strchr(QUOT, c) != NULL) {
                /* Adds the previous word to array */
                if (sait_cmp(begin, i)) _strarr_addn(&outarr, inarr, begin, i, 1, NULL);
                /* Sets quot flag on c value */
                quot = c;
                /* Moves to the next word (moves to the next character and set begin marker on it) */
                sait_incr(inarr, i, 1);
                sait_asgn(begin, i);
            } else if (c == L_BRACKET) {
                /* Adds the previous word to array */
                if (sait_cmp(begin, i)) _strarr_addn(&outarr, inarr, begin, i, 1, NULL);
                /* Adds "(" to array */
                _strarr_addn(&outarr, inarr, i, (sait_asgn(tmp, i), sait_incr(inarr, tmp, 1), tmp), 0, NULL);
                /* Moves to the next word (moves to the next character and set begin marker on it) */
                sait_incr(inarr, i, 1);
                sait_asgn(begin, i);
            } else if (c == R_BRACKET) {
                /* Adds the previous word to array */
                if (sait_cmp(begin, i)) _strarr_addn(&outarr, inarr, begin, i, 1, NULL);
                /* Adds ")" to array */
                _strarr_addn(&outarr, inarr, i, (sait_asgn(tmp, i), sait_incr(inarr, tmp, 1), tmp), 0, NULL);
                /* Moves to the next word (moves to the next character and set begin marker on it) */
                sait_incr(inarr, i, 1);
                sait_asgn(begin, i);
            } else if (strchr(SPACES, c) != NULL) {
                /* Adds the previous word to array */
                if (sait_cmp(begin, i)) _strarr_addn(&outarr, inarr, begin, i, 1, NULL);
                /* Moves to the next word (moves to the next character and set begin marker on it) */
                sait_incr(inarr, i, 1);
                sait_asgn(begin, i);
            } else if (sait_rpos(inarr, i) != 1 && c == sait_cnext(inarr, i, 1) && strchr(DOUBLE, c) != NULL) {
                /* Adds the previous word to array */
                if (sait_cmp(begin, i)) _strarr_addn(&outarr, inarr, begin, i, 1, NULL);
                /* Adds double key character to array */
                _strarr_addn(&outarr, inarr, i, (sait_asgn(tmp, i), sait_incr(inarr, tmp, 2), tmp), 0, NULL);
                /* Moves to the next word (moves to the next-next character and set begin marker on it) */
                sait_incr(inarr, i, 2);
                sait_asgn(begin, i);
            } else if (strchr(UNARY, c) != NULL) {
                /* Adds the previous word to array */
                if (sait_cmp(begin, i)) _strarr_addn(&outarr, inarr, begin, i, 1, NULL);
                /* Adds unary key character to array */
                _strarr_addn(&outarr, inarr, i, (sait_asgn(tmp, i), sait_incr(inarr, tmp, 1), tmp), 0, NULL);
                /* Moves to the next word (moves to the next character and set begin marker on it) */
                sait_incr(inarr, i, 1);
                sait_asgn(begin, i);
            } else if (c == SLASH) {
                /* Moves to the next-next character */
                if (sait_rpos(inarr, i) != 1) {
                    sait_incr(inarr, i, 2);
                } else {
                    sait_incr(inarr, i, 1);
                }
            } else if (c == COMMENT) {
                /* Stops reading */
                break;
            } else {
                /* Moves to the next character */
                sait_incr(inarr, i, 1);
            }
        }
    }
    /* If there is one more word, then adds it to array */
    if (sait_cmp(begin, i)) {
        _strarr_addn(&outarr, inarr, begin, i, 1, NULL);
    }

    if (quot) {
        fprintf(stderr, "%s: lexycal error\n", BASH_NAME);
        strarr_del(&outarr);
        return strarr_init();
    }

    return outarr;
}

int
_ctg(const char *str)
{
    /* Categories:
     *  1: (
     *  2: )
     *  4: < > >>
     *  8: | || &&
     * 16: & ;
     * 32: other
     * */

    if (!strcmp(str, "(")) {
        return 1;
    } else if (!strcmp(str, ")")) {
        return 2;
    } else if (!strcmp(str, "<") || !strcmp(str, ">") || !strcmp(str, ">>")) {
        return 4;
    } else if (!strcmp(str, "|") || !strcmp(str, "||") || !strcmp(str, "&&")) {
        return 8;
    } else if (!strcmp(str, "&") || !strcmp(str, ";")) {
        return 16;
    } else {
        return 32;
    }
}

int
_check_syntax(strarr arr)
{
    /* Categories:
     *  1: (
     *  2: )
     *  4: < > >>
     *  8: | || &&
     * 16: & ;
     * 32: other
     *
     * Rules:
     *    First: | 1 - 4 - -- 32 |
     * After  1: | 1 - 4 - -- 32 |
     * After  2: | - 2 4 8 16 -- |
     * After  4: | - - - - -- 32 |
     * After  8: | 1 - 4 - -- 32 |
     * After 16: | 1 2 4 - -- 32 |
     * After 32: | - 2 4 8 16 32 |
     *     Last: | - 2 - - 16 32 |
     * + Bracket balance
     * */
    
    const int arr_size = strarr_len(arr);
    if (arr_size == 0) {
        return 0;
    }

    /* Checks the first character */
    const int ctg_first = _ctg(arr[0]);
    if (ctg_first & ~(1 + 4 + 32)) {
        return 1;
    }

    char brck = 0; /* Count of left brackets minus count of right brackets */
    int i = 0;
    while (i + 1 < arr_size) {
        const char *str_cur = arr[i];
        const char *str_nxt = arr[i + 1];
        const int ctg_cur = _ctg(str_cur);
        const int ctg_nxt = _ctg(str_nxt);
        if (    ctg_cur == 1  && ctg_nxt & ~(1 + 4 + 32) ||
                ctg_cur == 2  && ctg_nxt & ~(2 + 4 + 8 + 16) ||
                ctg_cur == 4  && ctg_nxt & ~(32) ||
                ctg_cur == 8  && ctg_nxt & ~(1 + 4 + 32) ||
                ctg_cur == 16 && ctg_nxt & ~(1 + 2 + 4 + 32) ||
                ctg_cur == 32 && ctg_nxt & ~(2 + 4 + 8 + 16 + 32)) {
            return 2;
        }
        if (ctg_cur == 1) {
            ++brck;
        } else if (ctg_cur == 2) {
            --brck;
        }
        if (brck < 0) {
            return 3;
        }
        ++i;
    }

    /* Checks the last character */
    const int ctg_last = _ctg(arr[i]);
    if (ctg_last & ~(2 + 16 + 32)) {
        return 4;
    }
    if (ctg_last == 2) {
        --brck;
    }
    if (brck != 0) {
        return 5;
    }

    return 0;
}


ShTree *
_st_create_one_tree(strarr arr, int *pos, strarr ends)
{
    strarr argv = strarr_init();
    char *infile = NULL;
    char *outfile = NULL;
    char outmode = OM_WR;
    short backgrnd = BG_OFF;
    ShTree *psubcmd = NULL;
    ShTree *pipe = NULL;
    ShTree *next = NULL;
    short nextmode = NM_ANY;

    const int arr_size = strarr_len(arr);
    int i = *pos + 1;
    while (i < arr_size) {
        if (strarr_find(ends, arr[i]) != -1) {
            break;
        } else if (strcmp(arr[i], "<") == 0) {
            infile = arr[i + 1];
            i += 2;
        } else if (strcmp(arr[i], ">") == 0) {
            outfile = arr[i + 1];
            outmode = OM_WR;
            i += 2;
        } else if (strcmp(arr[i], ">>") == 0) {
            outfile = arr[i + 1];
            outmode = OM_APP;
            i += 2;
        } else if (strcmp(arr[i], "|") == 0) {
            pipe = _st_create_one_tree(arr, &i, ENDS_PIPE);
        } else if (strcmp(arr[i], "&&") == 0) {
            next = _st_create_one_tree(arr, &i, ENDS_NEXTIF);
            nextmode = NM_SUC;
        } else if (strcmp(arr[i], "||") == 0) {
            next = _st_create_one_tree(arr, &i, ENDS_NEXTIF);
            nextmode = NM_ERR;
        } else if (strcmp(arr[i], ";") == 0) {
            break;
        } else if (strcmp(arr[i], "&") == 0) {
            backgrnd = BG_ON;
            break;
        } else if (strcmp(arr[i], "(") == 0) {
            psubcmd = _st_create_sub_and_next(arr, &i);
        } else if (strcmp(arr[i], ")") == 0) {
            break;
        } else {
            strarr_add(&argv, arr[i]);
            ++i;
        }
    }

    *pos = i;
    ShTree *res = st_create(argv, infile, outfile, outmode, backgrnd, psubcmd, pipe, next, nextmode);
    strarr_del(&argv);
    st_delete(psubcmd);
    st_delete(pipe);
    st_delete(next);
    return res;
}

ShTree *
_st_create_sub_and_next(strarr arr, int *pos)
{
    ShTree *tr;

    ShTree *tmp = _st_create_one_tree(arr, pos, ENDS_DFLT);
    if (*pos < strarr_len(arr) && strcmp(arr[*pos], ")") == 0) {
        ++(*pos);
        tr = tmp;
    } else if (*pos + 1 < strarr_len(arr)) {
        tr = st_init();
        tr->psubcmd = tmp;
        tr->next = _st_create_sub_and_next(arr, pos);
    } else {
        tr = tmp;
    }

    return tr;
}

ShTree *
st_build(strarr arr)
{
    /* Checks syntax */
    int syntax_code;
    if (syntax_code = _check_syntax(arr)) {
        fprintf(stderr, "%s: Invalid syntax: error code %d\n", BASH_NAME, syntax_code);
        return st_init();
    }

    int pos = -1;
    return _st_create_sub_and_next(arr, &pos);
}
