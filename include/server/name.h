#ifndef __SERVER_NAME_H__
#define __SERVER_NAME_H__

#include <stdint.h>

/**
 * @defgroup name
 * @ingroup server
 *
 * @{
 */

#define MAX_NAME_SIZE 64
#define MAX_NAMERECORD_NUM 16

typedef enum {
    NS_REGISTER,
    NS_WHOIS,
} NSRequestType;

typedef struct {
    NSRequestType type;
    char name[MAX_NAME_SIZE];
} NSRequest;

void name_server_root_task();

/** @}*/

#endif
