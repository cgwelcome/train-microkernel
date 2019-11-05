#ifndef __SERVER_RPS_H__
#define __SERVER_RPS_H__

#include <stdint.h>

/**
 * @defgroup rps
 * @ingroup server
 *
 * @{
 */

#define MAX_MATCH_NUM 128
#define RPS_SERVER_NAME "RPS"

typedef enum {
    RPS_UNUSED,
    RPS_SIGNONE,
    RPS_WAITBOTH,
    RPS_READYP1,
    RPS_READYP2,
    RPS_READYBOTH,
    RPS_ENDGAME,
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

/**
 * Creates a Rock Paper Server
 */
int CreateRPS();

/** @}*/

#endif /*__SERVER_RPS_H__*/
