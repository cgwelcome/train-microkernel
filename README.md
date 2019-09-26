## Quick Start

At the school Linux server:

``` bash
cd src && make
cp main.elf /u/cs452/tftp/ARM/<YOUR UW USERNAME>
```

At the RedBoot prompt:

``` bash
RedBoot> load -h 10.15.167.5 ARM/<YOUR UW USERNAME>/main.elf
RedBoot> go
```

## Team Members
- Xiule Lin <x77lin@edu.uwaterloo.ca>
- Charles Gao <c39gao@edu.uwaterloo.ca>
