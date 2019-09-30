## Kernel Peformance 

We wrote a user program in `kernperform.c` under application that will peform tasks based on some global constant

| Constant | Description |
| ------ | ------ |
| SRR_NUM | The number of send, receive, reply |
| RECEIVEFIRST | A 0/1 flag whether it is receive first, if not then sender first  |
| MESSAGESIZE | Message size in byte  |

We will 32-bit clock built-in inside the TS-7200 board. A time will be captured before entering the kernel main loop, and when the kernel is about to exit, we will take the time difference. We will perform more than 10000 SRR, and divide with the time the kernel to run the main loop. This number is high enough to take a good average time for each single SRR. There is a bit of overhead like task creating, and task cleanup at the end that will factor in that time. However, by taking a high number of SRR, the overhead will be insignificant.

| Optimization | Cache | R/S | Message Size | Kernel time (ms) | Time per SRR (micros)
| ------------ | ----  | --- | ------------ | -----------      |  ----
| OFF          |   OFF  | R  | 4 | 7055 | 705.5
| OFF          |   OFF  | R  | 64 | 14229 | 1422.9
| OFF          |   OFF  | R  | 256 | 37320 | 3732.0
| OFF          |   OFF  | S  | 4 | 7276 | 727.6
| OFF          |   OFF  | S  | 64 | 14552 | 1455.2
| OFF          |   OFF  | S  | 256 | 37584 | 3758.4
| OFF          |   ON  | R  | 4 | 481 | 48.1
| OFF          |   ON  | R  | 64 | 917 | 91.7
| OFF          |   ON  | R  | 256 | 2254 | 225.4
| OFF          |   ON  | S  | 4 | 501 | 50.1
| OFF          |   ON  | S  | 64 | 925 | 92.5
| OFF          |   ON  | S  | 256 | 2322 | 232.2
| ON          |   OFF  | R  | 4 | 4273 | 427.3
| ON          |   OFF  | R  | 64 | 7593 | 759.3
| ON          |   OFF  | R  | 256 | 18210 | 1821.0
| ON          |   OFF  | S  | 4 | 4413 | 441.3
| ON          |   OFF  | S  | 64 | 7737 | 773.7
| ON          |   OFF  | S  | 256 | 18350 | 1835.0
| ON          |   ON  | R  | 4 | 298 | 29.8
| ON          |   ON  | R  | 64 | 468 | 46.8
| ON          |   ON  | R  | 256 | 1007 | 100.7
| ON          |   ON  | S  | 4 | 308 | 30.8
| ON          |   ON  | S  | 64 | 476 | 47.6
| ON          |   ON  | S  | 256 | 1017 | 101.7

### Conclusion

Our hypothesis is that most of the time is being spent on scheduling algorithm
the next task to be ran
