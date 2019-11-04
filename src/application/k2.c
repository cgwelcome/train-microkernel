#include <stdlib.h>
#include <hardware/timer.h>
#include <server/name.h>
#include <server/rps.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>
#include <utils/bwio.h>

#define RPS_MAX_ROUNDS 6
#define RPS_CLIENT_NUM 6
static const char moves[3] = { 'R', 'P', 'S' };

#define OUTPUT
#ifdef OUTPUT
    static const char *response2str[6] = { "READY", "TIE", "WIN", "LOST", "OTHERQUIT", "ACKQUIT" };
#endif // OUTPUT

static void rps_setup(int tid) {
    RPSResponse response;
    RPSRequest request;
    request.type = RPS_SIGNUP;
    Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response));
    assert(response == RPS_READY);
}

static RPSResponse rps_play(int tid, char move) {
    RPSResponse response;
    RPSRequest request;
    request.type = RPS_PLAY;
    request.move = move;
    Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response));
    return response;
}

static RPSResponse rps_quit(int tid)  {
    RPSResponse response;
    RPSRequest request;
    request.type = RPS_QUIT;
    Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response));
    return response;
}

static void rps_client() {
    RPSResponse response;
    int rpstid = WhoIs(RPS_SERVER_NAME);
    rps_setup(rpstid);
    srand((unsigned int) timer_read_raw(TIMER3));
    int rounds = rand() % (RPS_MAX_ROUNDS + 1);
#ifdef OUTPUT
    bwprintf(COM2, "RPS Client %d: Game Start. Want to play %d rounds\r\n", MyTid(), rounds);
#endif // OUTPUT
    for (int i = 0; i < rounds; i++) {
        char move = moves[rand() % 3];
        response = rps_play(rpstid, move);
        if (response == RPS_OTHERQUIT) {
#ifdef OUTPUT
            bwprintf(COM2, "RPS Client %d: Round %d, the other player quit.\r\n", MyTid(), i + 1);
#endif // OUTPUT
            break;
        }
#ifdef OUTPUT
        bwprintf(COM2, "RPS Client %d: Round %d, Move: %c, Result: %s\r\n", MyTid(), i + 1, move, response2str[response]);
        bwgetc(COM2);
#endif // OUTPUT
    }
    response = rps_quit(rpstid);
#ifdef OUTPUT
    bwprintf(COM2, "RPS Client %d: Quit with %s\r\n", MyTid(), response2str[response]);
#endif // OUTPUT
    Exit();
}

void k2_root_task() {
    bwsetfifo(COM2, OFF);
    CreateNameServer(2000);
    CreateRPS(2000);
    for (int i = 0; i < RPS_CLIENT_NUM; i++) {
        Create(500, &rps_client);
    }
    Exit();
}
