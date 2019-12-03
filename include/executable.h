#ifndef __EXECUTABLE_H__
#define __EXECUTABLE_H__

#include <train/track.h>

#include <stdint.h>

typedef struct {
    const char *name;
    void (*func)();
} Executable;

int exec_helloworld(int argc, char **argv);
int exec_argv(int argc, char **argv);
int exec_build_model(int argc, char **argv);
int exec_search_path(int argc, char **argv);
int exec_search_allpath(int argc, char **argv);
int exec_reverse(int argc, char **argv);
int exec_setup(int argc, char **argv);
int exec_collision(int argc, char **argv);
int exec_position(int argc, char **argv);
int exec_routing(int argc, char **argv);

void basic_track_setup();
void basic_setup();
void basic_menu(Executable *executable, uint32_t size);
void basic_train_setup(uint32_t train_id, TrackNode *node);

#endif /*__EXECUTABLE_H__*/
