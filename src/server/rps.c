#include <server/rps.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>

static uint32_t rpsmatch_count;
static RPSMatch rpsmatches[MAX_MATCH_NUM];

static void rps_signup(int tid) {
    if (rpsmatch_count == MAX_MATCH_NUM) {
        return;
    }
    if (rpsmatches[rpsmatch_count].status == RPS_UNUSED) {
        rpsmatches[rpsmatch_count].status = RPS_SIGNONE;
        rpsmatches[rpsmatch_count].p1tid = tid;
        return;
    }
    if (rpsmatches[rpsmatch_count].status == RPS_SIGNONE) {
        rpsmatches[rpsmatch_count].status = RPS_WAITBOTH;
        rpsmatches[rpsmatch_count].p2tid = tid;
    }
    RPSResponse request = RPS_READY;
    Reply(rpsmatches[rpsmatch_count].p1tid, (char *)&request, sizeof(request));
    Reply(rpsmatches[rpsmatch_count].p2tid, (char *)&request, sizeof(request));
    rpsmatch_count++;
}

// Return values:
//  2: Move2 wins
//  1: Move1 wins
//  0: Tie
static int rps_decide(char move1, char move2) {
    if (move1 == move2) {
        return 0;
    }
    if ((move1 == 'R' && move2 == 'S') ||
        (move1 == 'P' && move2 == 'R') ||
        (move1 == 'S' && move2 == 'P')) {
        return 1;
    } else {
        return 2;
    }
}

static void rps_matchupdate(RPSMatch *rpsmatch, int tid, char move) {
    if (rpsmatch->status == RPS_ENDGAME) {
        RPSResponse response = RPS_OTHERQUIT;
        Reply(tid, (char *)&response, sizeof(response));
        return;
    }
    if (rpsmatch->p1tid == tid) {
        rpsmatch->p1move = move;
        if (rpsmatch->status == RPS_READYP2) {
            rpsmatch->status = RPS_READYBOTH;
        } else {
            rpsmatch->status = RPS_READYP1;
        }
    } else {
        rpsmatch->p2move = move;
        if (rpsmatch->status == RPS_READYP1) {
            rpsmatch->status = RPS_READYBOTH;
        } else {
            rpsmatch->status = RPS_READYP2;
        }
    }
    if (rpsmatch->status == RPS_READYBOTH) {
        RPSResponse P1Response;
        RPSResponse P2Response;
        int decision = rps_decide(rpsmatch->p1move, rpsmatch->p2move);
        switch (decision) {
            case 0:
                P1Response = RPS_TIE;
                P2Response = RPS_TIE;
                break;
            case 1:
                P1Response = RPS_WIN;
                P2Response = RPS_LOST;
                break;
            case 2:
                P1Response = RPS_LOST;
                P2Response = RPS_WIN;
                break;
        }
        Reply(rpsmatch->p1tid, (char *)&P1Response, sizeof(P1Response));
        Reply(rpsmatch->p2tid, (char *)&P2Response, sizeof(P2Response));
        rpsmatch->status = RPS_WAITBOTH;
    }
}

static void rps_play(int tid, char move) {
    for (uint32_t i = 0; i < rpsmatch_count; i++) {
        if (rpsmatches[i].p1tid == tid || rpsmatches[i].p2tid == tid) {
            rps_matchupdate(&rpsmatches[i], tid, move);
            return;
        }
    }
}

static void rps_quit(int tid) {
    for (uint32_t i = 0; i < rpsmatch_count; i++) {
        if (rpsmatches[i].p1tid == tid || rpsmatches[i].p2tid == tid) {
            if (rpsmatches[i].status == RPS_READYP1) {
                RPSResponse response = RPS_OTHERQUIT;
                Reply(rpsmatches[i].p1tid, (char *)&response, sizeof(response));
            }
            if (rpsmatches[i].status == RPS_READYP2) {
                RPSResponse response = RPS_OTHERQUIT;
                Reply(rpsmatches[i].p2tid, (char *)&response, sizeof(response));
            }
            RPSResponse response = RPS_ACKQUIT;
            Reply(tid, (char *)&response, sizeof(response));
            rpsmatches[i].status = RPS_ENDGAME;
            return;
        }
    }
}

static void rsp_task() {
    int tid;
    RPSRequest request;

    rpsmatch_count = 0;
    for (uint32_t i = 0; i < MAX_MATCH_NUM; i++) {
        rpsmatches[i].status = RPS_UNUSED;
    }

    RegisterAs("RPS");
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        switch (request.type) {
            case RPS_SIGNUP:
                rps_signup(tid);
                break;
            case RPS_PLAY:
                rps_play(tid, request.move);
                break;
            case RPS_QUIT:
                rps_quit(tid);
                break;
            default:
                break;
        }
    }
}

int CreateRPS(uint32_t priority) {
    return Create(priority, &rsp_task);
}
