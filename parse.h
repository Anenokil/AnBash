#ifndef PARSE_H
#define PARSE_H

/* The function parses strarr */
char ** parse(char **strarr);

/* The function creates and returns ShTree by parsed array */
ShTree * st_build(char **arr);

#endif
