#ifndef STRARR_H
#define STRARR_H

#define FRMT_W "%s "
#define FRMT_L "%s\n"

typedef char **strarr; /* Array of strings */

/* Creates empty array and returns it */
strarr strarr_init(void);

/* Adds string 'str' to array '*parr' */
void strarr_add(strarr *parr, const char *str);

/* Returns number of elements in array 'arr' */
int strarr_len(strarr arr);

/* Prints array 'arr' using given format 'format' for each element */
void strarr_print(strarr arr, const char *format);

/* Creates a copy of array 'arr' and returns it */
strarr strarr_cp(strarr arr);

/* Finds string 'str' in array 'arr';
 * If there is 'str' in 'arr', returns index of it;
 * Otherwise returns -1 */
int strarr_find(strarr arr, const char *str);

/* Deletes array '*parr' and sets it to NULL */
void strarr_del(strarr *parr);

#endif
