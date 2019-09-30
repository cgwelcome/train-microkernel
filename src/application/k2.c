#include <stdlib.h>
#include <utils/kassert.h>
#include <utils/bwio.h>
#include <utils/timer.h>
#include <user/ipc.h>
#include <user/ns.h>
#include <user/tasks.h>
#include <server/ns.h>
#include <server/rps.h>

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
    kassert(response == RPS_READY);
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
    int rpstid = WhoIs("RPS");
    rps_setup(rpstid);
    srand(timer_read_raw());
    int rounds = rand() % (RPS_MAX_ROUNDS + 1);
#ifdef OUTPUT
    bwprintf(COM2, "RPS Client %d: Game Start. Want to play %d rounds\n\r", MyTid(), rounds);
#endif // OUTPUT
    for (int i = 0; i < rounds; i++) {
        char move = moves[rand() % 3];
        response = rps_play(rpstid, move);
        if (response == RPS_OTHERQUIT) {
#ifdef OUTPUT
            bwprintf(COM2, "RPS Client %d: Round %d, the other player quit.\n\r", MyTid(), i + 1);
#endif // OUTPUT
            break;
        }
#ifdef OUTPUT
        bwprintf(COM2, "RPS Client %d: Round %d, Move: %c, Result: %s\n\r", MyTid(), i + 1, move, response2str[response]);
        bwgetc(COM2);
#endif // OUTPUT
    }
    response = rps_quit(rpstid);
#ifdef OUTPUT
    bwprintf(COM2, "RPS Client %d: Quit with %s\n\r", MyTid(), response2str[response]);
#endif // OUTPUT
    Exit();
}

void k2_root_task() {
    bwsetfifo(COM2, OFF);
    CreateNS(2000);
    CreateRPS(2000);
    for (int i = 0; i < RPS_CLIENT_NUM; i++) {
        Create(500, &rps_client);
    }
    Exit();
}
