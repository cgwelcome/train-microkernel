/*
 * assert.h - provides debug assertion based on busy-wait I/O.
 */

#ifndef __UTILS_ASSERT_H__
#define __UTILS_ASSERT_H__

#include <arm.h>
#include <kernel.h>

#include <stdnoreturn.h>

noreturn void panic(char *file, int line, char *fmt, ...);

#define assert(expr) \
    if (!(expr)) panic(__FILE__, __LINE__, "Assertion failed: "#expr)

#define throw(...) \
    panic(__FILE__, __LINE__, __VA_ARGS__)

#endif
