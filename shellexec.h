#ifndef SHELLEXEC_H
#define SHELLEXEC_H

/* The function executes shell commands of tree and returns exit status;
 * 'emerg' is a function that is called in son after fork if execution is failed */
int shell_exec(ShTree *tree, int bg_pp, void (*emerg)(void));

#endif
