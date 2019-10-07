/*
 * assert.h - provides debug assertion based on busy-wait I/O.
 */

#ifndef __UTILS_KASSERT_H__
#define __UTILS_KASSERT_H__

#include <utils/bwio.h>

#define kassert(expr) \
    if (!(expr)) bwprintf(COM2, "%s: %d: Assertion '%s' failed.\n\r", __FILE__, __LINE__, #expr)

#endif
