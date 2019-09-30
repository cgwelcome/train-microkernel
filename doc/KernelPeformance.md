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
| OFF          |   OFF  | R  | 4 | 226553 | 2265.53
| OFF          |   OFF  | R  | 64 | 226492 | 2264.92
| OFF          |   OFF  | R  | 256 | 226495 | 2264.95
| OFF          |   OFF  | S  | 4 | 229072 | 2290.72
| OFF          |   OFF  | S  | 64 | 229242 | 2292.42
| OFF          |   OFF  | S  | 256 | 229234 | 2292.34
| OFF          |   ON  | R  | 4 | 16367 | 163.67
| OFF          |   ON  | R  | 64 | 16353 | 163.53
| OFF          |   ON  | R  | 256 | 16363 | 163.63
| OFF          |   ON  | S  | 4 | 16546 | 165.46
| OFF          |   ON  | S  | 64 | 16562 | 165.62
| OFF          |   ON  | S  | 256 | 16539 | 165.39
| ON          |   OFF  | R  | 4 | 119801 | 1198.01
| ON          |   OFF  | R  | 64 |  119840 | 1198.40
| ON          |   OFF  | R  | 256 | 120057 | 1200.57
| ON          |   OFF  | S  | 4 | 121151 | 1211.51
| ON          |   OFF  | S  | 64 | 121195 | 1211.95
| ON          |   OFF  | S  | 256 | 121431 | 1214.31
| ON          |   ON  | R  | 4 | 8309 | 83.09
| ON          |   ON  | R  | 64 | 8304 | 83.04
| ON          |   ON  | R  | 256 | 8313 | 83.13
| ON          |   ON  | S  | 4 | 8400 | 84.00
| ON          |   ON  | S  | 64 | 8403 | 84.03
| ON          |   ON  | S  | 256 | 8404 | 84.04
