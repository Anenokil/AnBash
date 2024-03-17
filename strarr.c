#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "strarr.h"

char **
strarr_init(void)
{
    char **arr = calloc(1, sizeof(*arr));
    arr[0] = NULL; /* Adds array terminator */
    return arr;
}

void
strarr_add(char ***parr, const char *str)
{
    assert(parr != NULL);
    assert(*parr != NULL);
    assert(str != NULL);

    char **arr = *parr;
    const int arr_len = strarr_len(arr);
    /* Creates tmp array with one more element then 'arr' has */
    char **tmp = calloc(arr_len + 2, sizeof(*tmp));
    /* Copies 'arr' to 'tmp' */
    int i;
    for (i = 0; i < arr_len; ++i) {
        const int str_len = strlen(arr[i]);
        tmp[i] = calloc(str_len + 1, sizeof(**tmp));
        strcpy(tmp[i], arr[i]);
    }
    /* Adds new element */
    tmp[arr_len] = calloc(strlen(str) + 1, sizeof(**tmp));
    strcpy(tmp[arr_len], str);
    /* Adds array terminator */
    tmp[arr_len + 1] = NULL;
    /* Replaces '*parr' by 'tmp' */
    strarr_del(parr);
    *parr = tmp;
}

int
strarr_len(char **arr)
{
    assert(arr != NULL);

    register int count = -1;
    while (arr[++count] != NULL);
    return count;
}

void
strarr_print(char **arr, const char *format)
{
    assert(arr != NULL);

    /* If 'format' is NULL, then uses default format */
    if (format == NULL) {
        format = FRMT_L;
    }

    register int i = 0;
    while (arr[i] != NULL) {
        printf(format, arr[i++]);
    }
}

char **
strarr_cp(char **arr)
{
    assert(arr != NULL);

    const int arr_len = strarr_len(arr);
    /* Creates new array */
    char **new_arr = calloc(arr_len + 1, sizeof(*new_arr));
    /* Copies elements */
    int i;
    for (i = 0; i < arr_len; ++i) {
        const int str_len = strlen(arr[i]);
        new_arr[i] = calloc(str_len + 1, sizeof(**new_arr));
        strcpy(new_arr[i], arr[i]);
    }
    /* Adds array terminator */
    new_arr[arr_len] = NULL;

    return new_arr;
}

int
strarr_find(char **arr, const char *str)
{
    assert(arr != NULL);
    assert(str != NULL);

    register int i = 0;
    while (arr[i] != NULL) {
        if (strcmp(arr[i], str) == 0) {
            return i;
        }
        ++i;
    }
    return -1;
}

void
strarr_del(char ***parr)
{
    assert(parr != NULL);
    assert(*parr != NULL);

    char **arr = *parr;
    register int i = 0;
    while (arr[i] != NULL) {
        free(arr[i++]);
    }
    free(arr);
    *parr = NULL;
}
