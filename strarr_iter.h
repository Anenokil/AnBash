/* The module implements string array iterator */
#ifndef STRARR_ITER_H
#define STRARR_ITER_H

typedef int sait[2]; /* Iterator: sait[0] is a number of a string, sait[1] is a number of a symbol in the string */

/* The function compares two sait variables;
 * if a = b, returns 0,
 * if a > b, returns 1,
 * if a < b, returns -1 */
int sait_cmp(const sait a, const sait b);

/* The function assigns the value 'src' to the variable 'dst' */
void sait_asgn(sait dst, const sait src);

/* The function increments sait on 'offset' value */
void sait_incr(strarr arr, sait cnt, unsigned offset);

/* The function returns a number of elements following the given */
int sait_rpos(strarr arr, sait pos);

/* The function returns a symbol of strarr with given position */
char sait_ccur(strarr arr, const sait pos);

/* The function returns a symbol of strarr with given position + offset */
char sait_cnext(strarr arr, const sait pos, unsigned offset);

#endif
