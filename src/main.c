#include <ts7200.h>
#include <kernel.h>
#include <tasks.h>

TaskGroup tasks;

void initialize() {
  // TODO: create first user task
  return;
}

void kmain() {
  initialize();  // includes starting the first user task
  for (;;) {
    int nextTID = Schedule(&tasks);
    int request = Activate(nextTID);
    switch (request) {
    default:
      break;
    }
  }
}
