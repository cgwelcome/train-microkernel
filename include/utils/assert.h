/*
 * assert.h - provides debug assertion based on busy-wait I/O.
 */

#ifndef __UTILS_ASSERT_H__
#define __UTILS_ASSERT_H__

#include <bwio.h>

#define assert(expr) \
    if (!(expr)) bwprintf(COM2, "%s: %d: Assertion '%s' failed.\n", __FILE__, __LINE__, #expr)

#endif // __UTILS_ASSERT_H__
