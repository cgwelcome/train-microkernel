#include <train/trainset.h>
#include <user/io.h>

void trainset_go(TrainIO *io) {
    Putc(io->tid, io->uart, TRAINSET_GO);
}

void trainset_stop(TrainIO *io) {
	Putc(io->tid, io->uart, TRAINSET_STOP);
}

void trainset_speed(TrainIO *io, uint32_t train_id, uint32_t speed) {
	Printf(io->tid, io->uart, "%c%c", speed, train_id);
}

void trainset_reverse(TrainIO *io, uint32_t train_id, uint32_t speed) {
	trainset_speed(io, train_id, TRAIN_REVERSE);
	trainset_speed(io, train_id, speed);
}

void trainset_switch(TrainIO *io, uint32_t switch_id, uint32_t status) {
	Printf(io->tid, io->uart, "%c%c", status, switch_id);
}

void trainset_switch_done(TrainIO *io) {
    Putc(io->tid, io->uart, (char)TRAINSWITCH_DONE);
}

static void trainset_sensor_parsemodule(ActiveTrainSensorList *sensorlist, char module, uint16_t raw) {
	for (uint32_t i = 0; i < MAX_SENSOR_PER_MODULE; i++) {
		if (raw % 2 == 1) {
			TrainSensor sensor = {
				.id = MAX_SENSOR_PER_MODULE - i,
				.module = module,
			};
			sensorlist->sensors[sensorlist->size] = sensor;
			sensorlist->size++;
		}
		raw = raw >> 1;
	}
}

ActiveTrainSensorList trainset_sensor_readall(TrainIO *io) {
	ActiveTrainSensorList sensorlist;
	sensorlist.size = 0;
	Putc(io->tid, io->uart, (char)(TRAINSENSOR_ALL + MODULE_TOTAL_NUM));
	for (char module = 'A'; module < 'A' + MODULE_TOTAL_NUM; module++) {
		uint8_t first = (uint8_t) Getc(io->tid, io->uart);
		uint8_t second = (uint8_t) Getc(io->tid, io->uart);
		uint16_t raw = (uint16_t)((first << sizeof(second)*8) | second);
		trainset_sensor_parsemodule(&sensorlist, module, raw);
	}
	return sensorlist;
}
