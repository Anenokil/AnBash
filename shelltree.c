#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "colors.h"
#include "strarr.h"
#include "shelltree.h"

enum
{
    TAB_SIZE = 4, /* Size of tabulation for 'st_print' function */
};

/* Colors for tree print function */
const char *CLR_DATA = CLR_G;
const char *CLR_TAB  = CLR_C;
const char *FRMT_ARGV = "[\033[033m%s\033[0m]";

/* Allocates and returns a copy of a string */
char * _strcopy(char *str);

/* Prints colored tabulation */
void _tab(char *tb);

/* Prints ShTree with given tabulation */
void _st_print(ShTree *tree, int to_print_all, int tabs);

char *
_strcopy(char *str)
{
    char *copy = calloc(strlen(str) + 1, sizeof(*str));
    strcpy(copy, str);
    return copy;
}

ShTree *
st_create(char **argv, char *infile, char *outfile, char outmode, short backgrnd,
        ShTree *psubcmd, ShTree *pipe, ShTree *next, short nextmode)
{
    ShTree *st = calloc(1, sizeof(*st));

    st->argv     =     argv == NULL ? NULL : strarr_cp(argv);
    st->infile   =   infile == NULL ? NULL : _strcopy(infile);
    st->outfile  =  outfile == NULL ? NULL : _strcopy(outfile);
    st->outmode  = outmode;
    st->backgrnd = backgrnd;
    st->pipe     =     pipe == NULL ? NULL : st_copy(pipe);
    st->psubcmd  =  psubcmd == NULL ? NULL : st_copy(psubcmd);
    st->next     =     next == NULL ? NULL : st_copy(next);
    st->nextmode = nextmode;

    return st;
}

ShTree *
st_init(void)
{
    strarr void_arr = strarr_init();
    ShTree *tmp = st_create(void_arr, NULL, NULL, OM_WR, BG_OFF, NULL, NULL, NULL, NM_ANY);
    strarr_del(&void_arr);
    return tmp;
}

ShTree *
st_copy(ShTree *tree)
{
    assert(tree != NULL);

    return st_create(tree->argv, tree->infile, tree->outfile, tree->outmode, tree->backgrnd,
            tree->psubcmd, tree->pipe, tree->next, tree->nextmode);
}

void
_tab(char *tb)
{
    printf("%s%s%s", CLR_TAB, tb, CLR_0);
}

void
_st_print(ShTree *tree, int to_print_all, int tabs)
{
    /* Creates tabulation strings */
    char *tb = calloc(tabs * TAB_SIZE + 1, sizeof(*tb));
    char *tb2 = calloc((tabs + 1) * TAB_SIZE + 1, sizeof(*tb2));
    for (int i = 0; i < tabs; ++i) {
        tb [i * TAB_SIZE] = ':';
        tb2[i * TAB_SIZE] = ':';
        for (int j = 1; j < TAB_SIZE; ++j) {
            tb [i * TAB_SIZE + j] = ' ';
            tb2[i * TAB_SIZE + j] = ' ';
        }
    }
    tb2[tabs * TAB_SIZE] = ':';
    for (int j = 1; j < TAB_SIZE; ++j) {
        tb2[tabs * TAB_SIZE + j] = ' ';
    }

    if (tree == NULL) {
        printf("NULL\n");
    } else {
        printf("{\n");

        _tab(tb2);
        printf("argv: ");
        strarr_print(tree->argv, FRMT_ARGV);
        printf("\n");
        if (to_print_all) {
            _tab(tb2);
            printf("infile: %s%s%s\n", CLR_DATA, tree->infile == NULL ? "NULL" : tree->infile, CLR_0);
            _tab(tb2);
            printf("outfile: %s%s%s\n", CLR_DATA, tree->outfile == NULL ? "NULL" : tree->outfile, CLR_0);
            _tab(tb2);
            printf("outmode: %s%c%s\n", CLR_DATA, tree->outmode, CLR_0);
            _tab(tb2);
            printf("backgrnd: %s%hi%s\n", CLR_DATA, tree->backgrnd, CLR_0);
            _tab(tb2);
            printf("psubcmd: ");
            _st_print(tree->psubcmd, to_print_all, tabs + 1);
            _tab(tb2);
            printf("pipe: ");
            _st_print(tree->pipe, to_print_all, tabs + 1);
            _tab(tb2);
            printf("next: ");
            _st_print(tree->next, to_print_all, tabs + 1);
            _tab(tb2);
            printf("nextmode: %s%hi%s\n", CLR_DATA, tree->nextmode, CLR_0);
        } else {
            if (tree->infile != NULL) {
                _tab(tb2);
                printf("infile: %s%s%s\n", CLR_DATA, tree->infile, CLR_0);
            }
            if (tree->outfile != NULL) {
                _tab(tb2);
                printf("outfile: %s%s%s\n", CLR_DATA, tree->outfile, CLR_0);
                _tab(tb2);
                printf("outmode: %s%c%s\n", CLR_DATA, tree->outmode, CLR_0);
            }
            _tab(tb2);
            printf("backgrnd: %s%hi%s\n", CLR_DATA, tree->backgrnd, CLR_0);
            if (tree->psubcmd != NULL) {
                _tab(tb2);
                printf("psubcmd: ");
                _st_print(tree->psubcmd, to_print_all, tabs + 1);
            }
            if (tree->pipe != NULL) {
                _tab(tb2);
                printf("pipe: ");
                _st_print(tree->pipe, to_print_all, tabs + 1);
            }
            if (tree->next != NULL) {
                _tab(tb2);
                printf("next: ");
                _st_print(tree->next, to_print_all, tabs + 1);
                _tab(tb2);
                printf("nextmode: %s%hi%s\n", CLR_DATA, tree->nextmode, CLR_0);
            }
        }

        _tab(tb);
        printf("}\n");
    }

    free(tb);
    free(tb2);
}

void
st_print(ShTree *tree, int to_print_all)
{
    _st_print(tree, to_print_all, 0);
}

void
st_delete(ShTree *tree)
{
    if (tree == NULL) {
        return;
    }

    strarr_del(&(tree->argv));
    if (tree->infile != NULL) {
        free(tree->infile);
    }
    if (tree->outfile != NULL) {
        free(tree->outfile);
    }
    st_delete(tree->psubcmd);
    st_delete(tree->pipe);
    st_delete(tree->next);
    free(tree);
}
