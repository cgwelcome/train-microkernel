# Kernel Peformance 

We wrote a user program in `kernperform.c` under application that will peform tasks based on some global constant

| Constant | Description |
| ------ | ------ |
| SRR_NUM | The number of send, receive, reply |
| RECEIVEFIRST | A 0/1 flag whether it is receive first, if not then sender first  |
| MESSAGESIZE | Message size in byte  |

We will 32-bit clock built-in inside the TS-7200 board. A time will be captured before entering the kernel main loop, and when the kernel is about to exit, we will take the time difference. We will perform more than 100 000 SRR, and divide with the time the kernel to run the main loop. This number is high enough to take a good average time for each single SRR. There is a bit of overhead like task creating, and task cleanup at the end that will factor in that time. However, by taking a high number of SRR, the overhead will be insignificant.

| Optimization | Cache | R/S | Message Size | Kernel time (ms) | Time per SRR (micros)
| ------------ | ----  | --- | ------------ | -----------      |  ----
| OFF          |   OFF  | R  | 4 | 22655 | 2265.5
| OFF          |   OFF  | R  | 64 | 30999 | 3099.9
| OFF          |   OFF  | R  | 256 | 52491 | 5249.1
| OFF          |   OFF  | S  | 4 | 22907 | 2290.7
| OFF          |   OFF  | S  | 64 | 29693 | 2969.3
| OFF          |   OFF  | S  | 256 | 52772 | 5277.2
| OFF          |   ON  | R  | 4 | 1637 | 163.7
| OFF          |   ON  | R  | 64 | 2069 | 206.9
| OFF          |   ON  | R  | 256 | 3408 | 340.8
| OFF          |   ON  | S  | 4 | 1655 | 165.5
| OFF          |   ON  | S  | 64 | 2075 | 207.5
| OFF          |   ON  | S  | 256 | 3416 | 341.6
| ON          |   OFF  | R  | 4 | 11980 | 1198.0
| ON          |   OFF  | R  | 64 | 15082 | 1508.2
| ON          |   OFF  | R  | 256 | 25711 | 2571.1
| ON          |   OFF  | S  | 4 | 12115 | 1211.5
| ON          |   OFF  | S  | 64 | 15302 | 1530.2
| ON          |   OFF  | S  | 256 | 25858 | 2585.8
| ON          |   ON  | R  | 4 | 831 | 83.1
| ON          |   ON  | R  | 64 | 1012 | 101.2
| ON          |   ON  | R  | 256 | 1550 | 155.0
| ON          |   ON  | S  | 4 | 840 | 84.0
| ON          |   ON  | S  | 64 | 999 | 99.9
| ON          |   ON  | S  | 256 | 1541 | 154.1
