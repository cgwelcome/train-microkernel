#include <server/rps.h>
#include <user/ipc.h>
#include <user/ns.h>
#include <user/tasks.h>

static unsigned int rpsmatch_count;
static RPSMatch rpsmatches[MAX_MATCH_NUM];
static int rpswaittid;

static void rps_signup(int tid) {
    if (rpswaittid == -1) {
        rpswaittid = tid;
    }
    else {
        if (rpsmatch_count == MAX_MATCH_NUM) {
            return;
        }
        // Add match to rpsmatches
        rpsmatches[rpsmatch_count].p1tid = rpswaittid;
        rpsmatches[rpsmatch_count].p2tid = tid;
        rpsmatches[rpsmatch_count].status= RPS_WAITBOTH;
        rpsmatch_count++;
        RPSResponse request = RPS_READY;
        Reply(tid, (char *)&request, sizeof(request));
        Reply(rpswaittid, (char *)&request, sizeof(request));
        rpswaittid = -1;
    }
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
    }
    else {
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
        rpsmatch->status |= RPS_READYP1;
    }
    else if (rpsmatch->p2tid == tid) {
        rpsmatch->p2move = move;
        rpsmatch->status |= RPS_READYP2;
    }
    if (rpsmatch->status == RPS_READYBOTH) {
        RPSResponse P1Response;
        RPSResponse P2Response;
        int decision = rps_decide(rpsmatch->p1move, rpsmatch->p2move);
        if (decision == 0) {
            P1Response = RPS_TIE;
            P2Response = RPS_TIE;
        }
        else if (decision == 1) {
            P1Response = RPS_WIN;
            P2Response = RPS_LOST;
        }
        else {
            P1Response = RPS_LOST;
            P2Response = RPS_WIN;
        }
        Reply(rpsmatch->p1tid, (char *)&P1Response, sizeof(P1Response));
        Reply(rpsmatch->p2tid, (char *)&P2Response, sizeof(P2Response));
        rpsmatch->status = RPS_WAITBOTH;
    }
}

static void rps_play(int tid, char move) {
    for (unsigned int i = 0; i < rpsmatch_count; i++) {
        if (rpsmatches[i].p1tid == tid || rpsmatches[i].p2tid == tid) {
            rps_matchupdate(&rpsmatches[i], tid, move);
            return;
        }
    }
}

static void rps_quit(int tid) {
    for (unsigned int i = 0; i < rpsmatch_count; i++) {
        if (rpsmatches[i].p1tid == tid || rpsmatches[i].p2tid == tid) {
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
    rpswaittid = -1;

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

int CreateRPS(unsigned int priority) {
    return Create(priority, &rsp_task);
}
