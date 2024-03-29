#include <string.h>
#include <assert.h>
#include "strarr.h"
#include "strarr_iter.h"

int
sait_cmp(const sait a, const sait b)
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
sait_asgn(sait dst, const sait src)
{
    dst[0] = src[0];
    dst[1] = src[1];
}

void
sait_incr(strarr arr, sait cnt, unsigned offset)
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
sait_rpos(strarr arr, sait pos)
{
    assert(arr != NULL);

    const sait end = { strarr_len(arr), 0 };
    if (sait_cmp(pos, end) >= 0) {
        return 0;
    }
    sait tmp;
    sait_asgn(tmp, pos);
    register int count = 0;
    while (sait_cmp(tmp, end)) {
        sait_incr(arr, tmp, 1);
        ++count;
    }
    return count;
}

char
sait_ccur(strarr arr, const sait pos)
{
    return arr[pos[0]][pos[1]];
}

char
sait_cnext(strarr arr, const sait pos, unsigned offset)
{
    sait nxt;
    sait_asgn(nxt, pos);
    sait_incr(arr, nxt, offset);
    return sait_ccur(arr, nxt);
}
