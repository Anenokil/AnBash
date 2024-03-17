#ifndef STRARR_COUNTER_H
#define STRARR_COUNTER_H

typedef int scnt[2]; /* Counter: scnt[0] is a number of string, scnt[1] is a number of symbol in the string */

/* The function compares two scnt variables;
 * if a = b, returns 0,
 * if a > b, returns 1,
 * if a < b, returns -1 */
int scnt_cmp(const scnt a, const scnt b);

/* The function assigns the value 'src' to the variable 'dst' */
void scnt_asgn(scnt dst, const scnt src);

/* The function increments scnt on 'offset' value */
void scnt_incr(strarr arr, scnt cnt, unsigned offset);

/* The function returns a number of elements following the given */
int scnt_rpos(strarr arr, scnt pos);

/* The function returns a symbol of strarr with given position */
char scnt_ccur(strarr arr, const scnt pos);

/* The function returns a symbol of strarr with given position + offset */
char scnt_cnext(strarr arr, const scnt pos, unsigned offset);

#endif
