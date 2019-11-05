#include <server/name.h>
#include <server/rps.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>
#include <utils/bwio.h>

static RPSResponse rps_quit(int tid)  {
    RPSResponse response;
    RPSRequest request;
    request.type = RPS_QUIT;
    assert(Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response)) >= 0);
    return response;
}

static RPSResponse rps_sendmove(int tid, char move) {
    RPSResponse response;
    RPSRequest request;
    request.type = RPS_PLAY;
    request.move = move;
    assert(Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response)) >= 0);
    return response;
}

static void rps_setup(int tid) {
    RPSResponse response;
    RPSRequest request;
    request.type = RPS_SIGNUP;
    assert(Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response)) >= 0);
    assert(response == RPS_READY);
}

static void rpstest_client1() {
    RPSResponse response;
    int rpstid = WhoIs("RPS");
    rps_setup(rpstid);
    response = rps_sendmove(rpstid, 'P');
    assert(response == RPS_TIE);
    response = rps_sendmove(rpstid, 'P');
    assert(response == RPS_LOST);
    response = rps_sendmove(rpstid, 'P');
    assert(response == RPS_WIN);
    response = rps_sendmove(rpstid, 'S');
    assert(response == RPS_LOST);
    response = rps_sendmove(rpstid, 'S');
    assert(response == RPS_OTHERQUIT);
    response = rps_quit(rpstid);
    assert(response == RPS_ACKQUIT);
    Exit();
}

static void rpstest_client2() {
    RPSResponse response;
    int rpstid = WhoIs("RPS");
    rps_setup(rpstid);
    response = rps_sendmove(rpstid, 'P');
    assert(response == RPS_TIE);
    response = rps_sendmove(rpstid, 'S');
    assert(response == RPS_WIN);
    response = rps_sendmove(rpstid, 'R');
    assert(response == RPS_LOST);
    response = rps_sendmove(rpstid, 'R');
    assert(response == RPS_WIN);
    response = rps_quit(rpstid);
    assert(response == RPS_ACKQUIT);
    Exit();
}

void rps_test_root_task() {
    CreateNameServer();
    CreateRPS();
    Create(1000, &rpstest_client1);
    Create(1000, &rpstest_client2);
    Exit();
}
