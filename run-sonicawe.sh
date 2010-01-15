#!/bin/bash
IS64=`if [ -n "\`uname -m | grep x86_64\`" ];then echo 64; fi`
export LD_LIBRARY_PATH=../misc:/usr/local/cuda/lib$IS64
echo $LD_LIBRARY_PATH
./sonicawe
# valgrind --leak-check=full --show-reachable=yes ./visualizer

