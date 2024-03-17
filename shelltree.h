#ifndef SHELLTREE_H
#define SHELLTREE_H

enum OUTMODES /* Values of ShTree.outmode */
{
    OM_WR = 'w', /* Write */
    OM_APP = 'a', /* Append */
};

enum BACKGRND /* Values of ShTree.backgrnd */
{
    BG_OFF = 0, /* Background mode is off */
    BG_ON = 1, /* Background mode is on */
};

enum NEXTMODES /* Values of ShTree.nextmode */
{
    NM_SUC = 1, /* Only if successful */
    NM_ERR = 2, /* Only in case of an error */
    NM_ANY = 3, /* Anyway */
};

typedef struct cmd_inf ShTree;
struct cmd_inf
{
    char **argv; /* Command and arguments */
    char *infile; /* Input file */
    char *outfile; /* Output file */
    char outmode; /* Open mode of output file */
    short backgrnd; /* Whether to execute in background mode */
    ShTree *psubcmd; /* Commands in brackets */
    ShTree *pipe; /* Pipe command */
    ShTree *next; /* Next command */
    short nextmode; /* Whether next command should be executed after success or fail */
};

/* Creates and returns ShTree with given field values */
ShTree * st_create(char **argv, char *infile, char *outfile, char outmode, short backgrnd,
        ShTree *psubcmd, ShTree *pipe, ShTree *next, short nextmode);

/* Creates and returns empty ShTree */
ShTree * st_init(void);

/* Returns a copy of ShTree */
ShTree * st_copy(ShTree *tree);

/* Prints ShTree
 * If to_print_all is set on 0, prints only non-empty fields of tree; otherwise prints all fields */
void st_print(ShTree *tree, int to_print_all);

/* Deletes ShTree */
void st_delete(ShTree *tree);

#endif
