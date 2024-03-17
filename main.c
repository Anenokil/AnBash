#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for pplr */
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/limits.h>
#include "colors.h"
#include "strarr.h"
#include "shelltree.h"
#include "shellexec.h"
#include "parse.h"

enum
{
    BUF_SIZE = 4, /* Size of input buffer */
    TESTBUF_SIZE = 4096, /* Size of buffer for test input */
    ARGC = 2, /* Expected number of console arguments */
    FLAGS_DFLT = 16, /* Default flags value (if flags value is not specified) */
};

const char *FRMT_ARR = "[\033[033m%s\033[0m]"; /* Format for array print */
const char BASH_NAME[] = "anbash";

/* Signal handler */
void sig_handler(int s);

/* Prompt to enter */
void prompt(void);

/* Frees global variables */
void free_mem(void);

/* What to do if execution in son is failed after fork */
void emerg_shutdown(void);

/* Forms strarr 'words' of lexemas of a tree and array 'counters' of their occurrences */
void pplr_rate(ShTree *tree, int *counters, strarr *words);

/* Finds the most popular lexema in a tree and returns it (free required);
 * in count variable it writes a number of occurrences of the lexema */
char * find_most_pplr(ShTree *tree, int *count);

/* Global variables */
strarr inp_arr = NULL;
strarr st_argv = NULL;
ShTree *st = NULL;
char *test_fn = NULL;
FILE *testfile = NULL;
char curdir[PATH_MAX];
int bg_pp[2];

int
main(int argc, char **argv)
{
    /* Sets signal handler for SIGINT */
    sigaction(SIGINT, &(struct sigaction){.sa_handler = sig_handler, .sa_flags = SA_RESTART}, NULL);

    /* Sets processing flags:
     * flags & 1  - to print input
     * flags & 2  - to print parsed input
     * flags & 4  - to print bash tree
     * flags & 8  - to print all fields in bash tree
     * flags & 16 - to execute commands
     * flags & 32 - to test program
     * */
    const short flags = argc < ARGC ? FLAGS_DFLT : atoi(argv[1]); /* If flags are not specified, sets default */
    const short to_print_input = flags & 1;
    const short to_print_pars = flags & 2;
    const short to_print_tree = flags & 4;
    const short to_print_full = flags & 8;
    const short to_exec = flags & 16;
    const short to_test = flags & 32;

    /* If test mode is enabled, scans filename and opens testfile */
    if (to_test) {
        test_fn = calloc(TESTBUF_SIZE, sizeof(*test_fn));
        while (testfile == NULL) {
            scanf("%s", test_fn);
            testfile = fopen(test_fn, "r");
        }
    }

    pipe(bg_pp);
    fcntl(bg_pp[0], F_SETFL, O_NONBLOCK, 1);
    fcntl(bg_pp[1], F_SETFL, O_NONBLOCK, 1);

    char buf[BUF_SIZE + 1] = { 0 };
    while (1) {
        /* Removes zombies */
        pid_t pid;
        while (read(bg_pp[0], &pid, sizeof(pid)) == sizeof(pid)) {
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 1);
        }
        
        /* Prompt to enter */
        prompt();
        inp_arr = strarr_init();

        if (to_test) { /* If test mode is enabled */
            /* Scans line from testfile */
            char buf[TESTBUF_SIZE];
            if (fgets(buf, TESTBUF_SIZE, testfile) == NULL) {
                /* If finds EOF, stops processing */
                free_mem();
                write(1, "\n", 1);
                _exit(0);
            }
            int size = strlen(buf);
            if (size > 1) {
                buf[size - 1] = '\0';
            }
            /* Prints current test input */
            printf("%s\n", buf);
            /* Adds string to array */
            strarr_add(&inp_arr, buf);
        } else { /* If test mode is disabled */
            /* Scans line in buffer by parts */
            int size = 0;
            while ((size = read(0, buf, BUF_SIZE)) == BUF_SIZE && buf[BUF_SIZE - 1] != '\n') {
                strarr_add(&inp_arr, buf);
            }
            /* Ctrl+D processing */
            if (size == 0) {
                free_mem();
                write(1, "\n", 1);
                _exit(0);
            }
            /* Adds string to array */
            if (size > 1) {
                buf[size - 1] = '\0';
                strarr_add(&inp_arr, buf);
            }
        }

        /* Prints buffered input */
        if (to_print_input) {
            printf("\n%sInput:%s\n", CLR_G, CLR_0);
            strarr_print(inp_arr, FRMT_ARR);
            printf("\n");
        }

        /* Parses input */
        st_argv = parse(inp_arr);
        strarr_del(&inp_arr);

        /* Prints parsed input */
        if (to_print_pars) {
            printf("\n%sParsed input:%s\n", CLR_G, CLR_0);
            strarr_print(st_argv, FRMT_ARR);
            printf("\n");
        }

        /* Creates tree */
        st = st_build(st_argv);
        strarr_del(&st_argv);

        /* Prints tree */
        if (to_print_tree) {
            printf("\n%sShell tree:%s ", CLR_G, CLR_0);
            st_print(st, to_print_full);
            /*
            int count;
            char *wrd = find_most_pplr(st, &count);
            printf("\nMost popular: %s (%d)\n", wrd, count);
            free(wrd);
            */
        }

        /* Executes commands */
        if (to_exec) {
            if (flags & 7) {
                printf("\n%sExecution:%s\n", CLR_G, CLR_0);
            }
            shell_exec(st, bg_pp[1], &emerg_shutdown);
        }

        st_delete(st);
        st = NULL;
    }
}

void
sig_handler(int s)
{
    if (s == SIGINT) {
        free_mem();
        write(1, "\n", 1);
        _exit(0);
    }
}

void
prompt(void) {
    getcwd(curdir, PATH_MAX);
    printf("%s%s%s:%s%s%s$ ", CLR_R, BASH_NAME, CLR_0, CLR_Y, curdir, CLR_0);
    fflush(stdout);
}

void
free_mem(void)
{
    if (inp_arr != NULL) {
        strarr_del(&inp_arr);
    }
    if (st_argv != NULL) {
        strarr_del(&st_argv);
    }
    if (st != NULL) {
        st_delete(st);
    }
    if (test_fn != NULL) {
        free(test_fn);
    }
    if (testfile != NULL) {
        fclose(testfile);
    }
    close(bg_pp[0]);
    close(bg_pp[1]);
}

void
emerg_shutdown(void)
{
    free_mem();
}

void
pplr_rate(ShTree *tree, int *counters, strarr *words)
{
    if (tree == NULL || tree->argv == NULL && tree->psubcmd == NULL) {
        return;
    }

    if (tree->argv != NULL) {
        for (int i = 0; (tree->argv)[i] != NULL; ++i) {
            int pos = strarr_find(*words, (tree->argv)[i]);
            if (pos == -1) {
                strarr_add(words, (tree->argv)[i]);
                ++counters[strarr_len(*words) - 1];
            } else {
                ++counters[pos];
            }
        }
    }

    pplr_rate(tree->psubcmd, counters, words);
    pplr_rate(tree->pipe, counters, words);
    pplr_rate(tree->next, counters, words);
}

char *
find_most_pplr(ShTree *tree, int *count)
{
    int counters[4096] = { 0 }; /**/
    strarr words = NULL;

    pplr_rate(tree, counters, &words);

    int max = 0;
    int max_index = 0;
    for (int i = 0; i < strarr_len(words); ++i) {
        if (counters[i] > max) {
            max = counters[i];
            max_index = i;
        }
    }

    *count = max;
    char *word = calloc(strlen(words[max_index]) + 1, sizeof(*word));
    strcpy(word, words[max_index]);
    strarr_del(&words);
    return word;
}
