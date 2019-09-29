#ifndef __SERVER_RPS_H__
#define __SERVER_RPS_H__

#define MAX_MATCH_NUM 128

typedef enum {
    RPS_UNUSED,
    RPS_SIGNONE,
    RPS_WAITBOTH,
    RPS_READYP1   = 0x10,
    RPS_READYP2   = 0x20,
    RPS_READYBOTH = 0x30,
    RPS_ENDGAME   = 0xFF,
} RPSMatchStatus;

typedef struct {
    int p1tid;
    char p1move;
    int p2tid;
    char p2move;
    RPSMatchStatus status;
} RPSMatch;

typedef enum {
    RPS_READY,
    RPS_TIE,
    RPS_WIN,
    RPS_LOST,
    RPS_OTHERQUIT,
    RPS_ACKQUIT,
} RPSResponse;

typedef enum {
    RPS_SIGNUP,
    RPS_PLAY,
    RPS_QUIT,
} RPSRequestType;

typedef struct {
    RPSRequestType type;
    char move;
} RPSRequest;

// CreateRPS() creates a Rock Paper Server
int CreateRPS(unsigned int priority);

#endif
