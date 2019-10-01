#ifndef __SERVER_NS_H__
#define __SERVER_NS_H__

#define MAX_NAME_SIZE 64
#define MAX_NAMERECORD_NUM 248

typedef enum {
    NS_REGISTER,
    NS_WHOIS,
} NSRequestType;

typedef struct {
    NSRequestType type;
    char name[MAX_NAME_SIZE];
} NSRequest;

// InitNS() initialize some global counters for the Name Server.
void InitNS();

// CreateNS() creates Name Server
int CreateNS(unsigned int priority);

#endif
