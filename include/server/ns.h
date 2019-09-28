#ifndef __SERVER_NS_H__
#define __SERVER_NS_H__

#define MAX_NAME_SIZE 64
#define MAX_NAMERECORD_NUM 248

int ns_tid;

typedef enum {
    NS_REGISTER,
    NS_WHOIS,
} NSRequestType;

typedef struct {
    NSRequestType type;
    char name[MAX_NAME_SIZE];
} NSRequest;

// CreateNS() creates Name Server
int CreateNS(unsigned int priority);

#endif
