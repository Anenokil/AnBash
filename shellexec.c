#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include "shelltree.h"

enum ERRORS
{
    ERR_FORK = 1,
    ERR_EXEC = 2,
    ERR_WAIT = 3,
    ERR_OPEN = 4,
    ERR_OUTMODE = 5,
};

extern const char BASH_NAME[];

/* The function closes file descriptor if it is open */
void _close_fd(int fd);

/* The function executes cmd with given input and output files or pipes */
void _exec_io(char **argv, int ipp, int opp, char *inf, char *outf, char outmode, void (*emerg)(void));

/* The function executes "cd" command */
int _cd(char **argv);

/* The function executes cmd with given input and output files or pipes and waits it */
int _syst(char **argv, int ipp, int opp, char *inf, char *outf, char outmode, void (*emerg)(void));

/* The function executes shell commands of tree and returns exit status;
 * 'emerg' is a function that is called in son after fork if execution is failed */
int _shell_exec(ShTree *tree, int ipp_ext, int opp_ext, char *inf_ext, char *outf_ext, char outmode_ext,
        int bg_pp, void (*emerg)(void));

void
_close_fd(int fd)
{
    if (fd != -1) {
        close(fd);
    }
}

void
_exec_io(char **argv, int ipp, int opp, char *inf, char *outf, char outmode, void (*emerg)(void))
{
    assert(argv != NULL);

    if (inf != NULL) {
        int infd = open(inf, O_RDONLY, 0444);
        if (infd == -1) {
            fprintf(stderr, "%s: %s: No such file or directory\n", BASH_NAME, inf);
            fflush(stderr);
            emerg();
            _exit(ERR_OPEN);
        }
        struct stat inf_stat;
        stat(inf, &inf_stat);
        if (S_ISDIR(inf_stat.st_mode)) {
            fprintf(stderr, "%s: %s: Is a directory\n", BASH_NAME, inf);
            fflush(stderr);
            emerg();
            _exit(ERR_OPEN);
        }
        dup2(infd, 0);
        close(infd); /**/
    } else if (ipp != -1) {
        dup2(ipp, 0);
    }
    _close_fd(ipp);
    
    if (outf != NULL) {
        int outfd;
        if (outmode == OM_WR) {
            outfd = open(outf, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        } else if (outmode == OM_APP) {
            outfd = open(outf, O_WRONLY | O_CREAT | O_APPEND, 0666);
        } else {
            _exit(ERR_OUTMODE);
        }
        struct stat outf_stat;
        stat(outf, &outf_stat);
        if (S_ISDIR(outf_stat.st_mode)) {
            fprintf(stderr, "%s: %s: Is a directory\n", BASH_NAME, outf);
            fflush(stderr);
            emerg();
            _exit(ERR_OPEN);
        }
        dup2(outfd, 1);
        close(outfd); /**/
    } else if (opp != -1) {
        dup2(opp, 1);
    }
    _close_fd(opp);

    execvp(argv[0], argv);
    fprintf(stderr, "%s: exec: error\n", BASH_NAME);
    fflush(stderr);
    emerg();
    _exit(ERR_EXEC);
}

int
_cd(char **argv)
{
    if (argv[1] == NULL || strcmp(argv[1], "~") == 0) {
        return chdir(getenv("HOME"));
    } else {
        return chdir(argv[1]);
    }
}

int
_syst(char **argv, int ipp, int opp, char *inf, char *outf, char outmode, void (*emerg)(void))
{
    pid_t frk = fork();
    if (frk < 0) {
        return ERR_FORK;
    } else if (!frk) {
        /* If command is cd */
        if (argv != NULL && strcmp(argv[0], "cd") == 0) {
            _close_fd(ipp);
            _close_fd(opp);
            return _cd(argv);
        }
        /* Otherwise */
        _exec_io(argv, ipp, opp, inf, outf, outmode, emerg);
    } else {
        _close_fd(opp);
        int st;
        if (wait(&st) == -1) {
            return ERR_WAIT;
        }
        if (!WIFEXITED(st) || WEXITSTATUS(st)) {
            return ERR_EXEC;
        }
        return 0;
    }
}

int
_shell_exec(ShTree *tree, int ipp_ext, int opp_ext, char *inf_ext, char *outf_ext, char outmode_ext,
        int bg_pp, void (*emerg)(void))
{
    /* Input-output priorities:
     * 1) tree->infile | tree->outfile
     * 2) pp[0]        | pp[1]
     * 3) inf_ext      | outf_ext
     * 4) ipp_ext      | opp_ext
     * */

    char *inf = tree->infile == NULL ? inf_ext : tree->infile;
    char *outf = tree->outfile == NULL ? outf_ext : tree->outfile;
    char outmode = tree->outfile == NULL ? outmode_ext : tree->outmode;

    int ret = 0;

    int frk1 = fork();
    if (frk1 < 0) {
        ret = ERR_FORK;
    } else if (!frk1) {
        /* Chanel for pipe command */
        int pp[2];
        pipe(pp);

        int frk = fork();
        if (frk < 0) {
            /* If has error */
            close(pp[0]);
            close(pp[1]);
            ret = ERR_FORK;
        } else if (!frk) {
            /* Son executes argv or psubcmd (exclude each other) */
            close(pp[0]);
            if (tree->argv != NULL && tree->argv[0] != NULL) {
                if (tree->pipe != NULL) {
                    ret = _syst(tree->argv, ipp_ext, pp[1], inf, tree->outfile, tree->outmode, emerg);
                } else {
                    ret = _syst(tree->argv, ipp_ext, opp_ext, inf, outf, outmode, emerg);
                }
            } else if (tree->psubcmd != NULL) {
                if (tree->pipe != NULL) {
                    ret = _shell_exec(tree->psubcmd, ipp_ext, pp[1], inf, tree->outfile, tree->outmode,
                            bg_pp, emerg);
                } else {
                    ret = _shell_exec(tree->psubcmd, ipp_ext, opp_ext, inf, outf, outmode, bg_pp, emerg);
                }
            }
            close(pp[1]);
            emerg(); /* Not emerg - just freemem */
            _exit(ret);
        } else {
            /* Father executes pipe */
            close(pp[1]);
            if (tree->pipe != NULL) {
                ret = _shell_exec(tree->pipe, pp[0], opp_ext, NULL, outf_ext, outmode_ext, bg_pp, emerg);
            }
            close(pp[0]);

            /* Waits son */
            int st;
            if (wait(&st) == -1) {
                ret = ERR_WAIT;
            }
            if (!WIFEXITED(st) || WEXITSTATUS(st)) {
                ret = ERR_EXEC;
            }
            
            /* Adds process to bg pipe */
            pid_t pid = getpid();
            write(bg_pp, &pid, sizeof(pid));

            emerg(); /* Not emerg - just freemem */
            _exit(ret);
        }
    } else {
        if (tree->backgrnd == BG_OFF) {
            int st;
            if (wait(&st) == -1) {
                ret = ERR_WAIT;
            }
            if (!WIFEXITED(st) || WEXITSTATUS(st)) {
                ret = ERR_EXEC;
            }
        }

        /* Moves to next */
        if (tree->next != NULL) {
            if (ret && tree->nextmode != NM_SUC || !ret && tree->nextmode != NM_ERR) {
                ret = _shell_exec(tree->next, ipp_ext, opp_ext, inf_ext, outf_ext, outmode_ext,
                        bg_pp, emerg);
            }
        }
    }
    return ret;
}

int
shell_exec(ShTree *tree, int bg_pp, void (*emerg)(void))
{
    return _shell_exec(tree, -1, -1, NULL, NULL, OM_WR, bg_pp, emerg);
}
