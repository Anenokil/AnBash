#include <string.h>
#include <assert.h>
#include "strarr.h"
#include "strarr_counter.h"

int
scnt_cmp(const scnt a, const scnt b)
{
    if (a[0] == b[0]) {
        if (a[1] == b[1]) {
            return 0;
        } else if (a[1] > b[1]) {
            return 1;
        } else {
            return -1;
        }
    } else if (a[0] > b[0]) {
        return 1;
    } else {
        return -1;
    }
}

void
scnt_asgn(scnt dst, const scnt src)
{
    dst[0] = src[0];
    dst[1] = src[1];
}

void
scnt_incr(strarr arr, scnt cnt, unsigned offset)
{
    assert(arr != NULL);

    /* Iterates by strings */
    register int len;
    while (arr[cnt[0]] != NULL && (len = strlen(arr[cnt[0]])) <= cnt[1] + offset) {
        offset -= len - cnt[1];
        ++cnt[0];
        cnt[1] = 0;
    }
    /* Offset in the last string */
    cnt[0] += (cnt[1] + offset) / len;
    cnt[1]  = (cnt[1] + offset) % len;
}

int
scnt_rpos(strarr arr, scnt pos)
{
    assert(arr != NULL);

    const scnt end = { strarr_len(arr), 0 };
    if (scnt_cmp(pos, end) >= 0) {
        return 0;
    }
    scnt tmp;
    scnt_asgn(tmp, pos);
    register int count = 0;
    while (scnt_cmp(tmp, end)) {
        scnt_incr(arr, tmp, 1);
        ++count;
    }
    return count;
}

char
scnt_ccur(strarr arr, const scnt pos)
{
    return arr[pos[0]][pos[1]];
}

char
scnt_cnext(strarr arr, const scnt pos, unsigned offset)
{
    scnt nxt;
    scnt_asgn(nxt, pos);
    scnt_incr(arr, nxt, offset);
    return scnt_ccur(arr, nxt);
}
