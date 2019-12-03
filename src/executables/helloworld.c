#include <kernel.h>
#include <user/name.h>
#include <user/io.h>

int exec_helloworld(int argc, char **argv) {
    (void)argc;
    (void)argv;
    int iotid = WhoIs(SERVER_NAME_IO);
    Printf(iotid, COM2, "hello world");
    return 0;
}
