#ifndef __SERVER_NAME_H__
#define __SERVER_NAME_H__

#define MAX_NAME_SIZE 64
#define MAX_NAMERECORD_NUM 248

#include <stdint.h>

typedef enum {
    NS_REGISTER,
    NS_WHOIS,
} NSRequestType;

typedef struct {
    NSRequestType type;
    char name[MAX_NAME_SIZE];
} NSRequest;

/**
 * Initialize some global counters for the Name Server.
 */
void InitNameServer();

/**
 * Creates a global Name Server
 */
int CreateNameServer(uint32_t priority);

#endif
