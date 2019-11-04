/*
 * assert.h - provides debug assertion based on busy-wait I/O.
 */

#ifndef __UTILS_ASSERT_H__
#define __UTILS_ASSERT_H__

#include <arm.h>
#include <stdint.h>
#include <kernel.h>
#include <utils/bwio.h>

void panic(char *expr, char *file, int line);

#define assert(expr) \
    if (!(expr)) panic(#expr, __FILE__, __LINE__)

#endif
