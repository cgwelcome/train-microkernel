#ifndef __TEST_H__
#define __TEST_H__

#include <stdint.h>

typedef struct {
    const char *name;
    void (*func)();
} TestCase;

int test_helloworld(int argc, char **argv);
int test_argv(int argc, char **argv);
int test_build_model(int argc, char **argv);
int test_search_path(int argc, char **argv);
int test_search_allpath(int argc, char **argv);
int test_reverse(int argc, char **argv);
int test_setup(int argc, char **argv);

void basic_setup();
void basic_menu(TestCase *suite, uint32_t size);

#endif /*__TEST_H__*/
