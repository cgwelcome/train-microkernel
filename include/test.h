#ifndef __TEST_H__
#define __TEST_H__

#define TRAIN_SPEED    10
#define TRAIN_ID       58
#define NODE_TRACK_A  "A8"
#define NODE_TRACK_B  "A13"

int test_helloworld(int argc, char **argv);
int test_argv(int argc, char **argv);
int test_build_model(int argc, char **argv);
int test_search_path(int argc, char **argv);
int test_search_allpath(int argc, char **argv);
int test_reverse(int argc, char **argv);
int test_setup(int argc, char **argv);

void basic_setup(int io_tid, int train_tid);

#endif /*__TEST_H__*/
