#include <user/clock.h>
#include <user/name.h>
#include <user/ipc.h>
#include <user/tasks.h>
#include <server/clock.h>
#include <server/trainmanager.h>
#include <train/notifier.h>

void trainnotifier_sensor_task() {
	int clocktid = WhoIs(CLOCK_SERVER_NAME);
	int traintid = WhoIs(TRAINMANAGER_SERVER_NAME);

    TMRequest request = {
        .type = TMREQUESTTYPE_UPDATE_STATUS,
    };

	for (;;) {
		Delay(clocktid, SENSOR_READ_INTERVAL);
		Send(traintid, (char *)&request, sizeof(request), NULL, 0);
	}
	Exit();
}
