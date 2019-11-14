#include <kernel.h>
#include <user/name.h>
#include <user/io.h>

int argv(int argc, char **argv) {
    (void)argc;
    (void)argv;
    int iotid = WhoIs(SERVER_NAME_IO);
    for (int i = 0; i < argc; i++) {
        Printf(iotid, COM2, "%s\n\r", argv[i]);
    }
    return 0;
}

