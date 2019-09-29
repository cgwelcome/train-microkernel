#include <utils/kassert.h>
#include <utils/bwio.h>
#include <user/ipc.h>
#include <user/ns.h>
#include <user/tasks.h>
#include <server/ns.h>
#include <server/rps.h>

RPSResponse rps_quit(int tid)  {
    RPSResponse response;
    RPSRequest request;
    request.type = RPS_QUIT;
    Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response));
    return response;
}

RPSResponse rps_sendmove(int tid, char move) {
    RPSResponse response;
    RPSRequest request;
    request.type = RPS_PLAY;
    request.move = move;
    Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response));
    return response;
}

void rps_setup(int tid) {
    RPSResponse response;
    RPSRequest request;
    request.type = RPS_SIGNUP;
    Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response));
    kassert(response == RPS_READY);
}

void rpstest_client1() {
    RPSResponse response;
    int rpstid = WhoIs("RPS");
    rps_setup(rpstid);
    response = rps_sendmove(rpstid, 'P');
    kassert(response == RPS_TIE);
    response = rps_sendmove(rpstid, 'P');
    kassert(response == RPS_LOST);
    response = rps_sendmove(rpstid, 'P');
    kassert(response == RPS_WIN);
    response = rps_sendmove(rpstid, 'S');
    kassert(response == RPS_LOST);
    response = rps_sendmove(rpstid, 'S');
    kassert(response == RPS_OTHERQUIT);
    response = rps_quit(rpstid);
    kassert(response == RPS_ACKQUIT);
    Exit();
}

void rpstest_client2() {
    RPSResponse response;
    int rpstid = WhoIs("RPS");
    rps_setup(rpstid);
    response = rps_sendmove(rpstid, 'P');
    kassert(response == RPS_TIE);
    response = rps_sendmove(rpstid, 'S');
    kassert(response == RPS_WIN);
    response = rps_sendmove(rpstid, 'R');
    kassert(response == RPS_LOST);
    response = rps_sendmove(rpstid, 'R');
    kassert(response == RPS_WIN);
    response = rps_quit(rpstid);
    kassert(response == RPS_ACKQUIT);
    Exit();
}



void rpstest_root_task() {
    CreateNS(2000);
    CreateRPS(2000);
    Create(1000, &rpstest_client1);
    Create(1000, &rpstest_client2);
    Exit();
}
