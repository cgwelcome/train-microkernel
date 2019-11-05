/*
 * assert.h - provides debug assertion based on busy-wait I/O.
 */

#ifndef __UTILS_ASSERT_H__
#define __UTILS_ASSERT_H__

#include <arm.h>
#include <stdint.h>
#include <stdnoreturn.h>
#include <kernel.h>
#include <utils/bwio.h>

noreturn void panic(char *expr, char *file, int line);

#define assert(expr) \
    if (!(expr)) panic("Assertion failed: "#expr, __FILE__, __LINE__)

#define throw(error_msg) \
    panic(error_msg, __FILE__, __LINE__)

#endif
