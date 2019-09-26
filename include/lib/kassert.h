/*
 * assert.h - provides debug assertion based on busy-wait I/O.
 */

#ifndef __UTILS_KASSERT_H__
#define __UTILS_KASSERT_H__

#include <bwio.h>

#define kassert(expr) \
    if (!(expr)) bwprintf(COM2, "%s: %d: Assertion '%s' failed.\n", __FILE__, __LINE__, #expr)

#endif // __UTILS_KASSERT_H__
