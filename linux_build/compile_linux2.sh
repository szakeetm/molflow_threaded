cd ../intermediary/linux
g++-8  -c -DMOLFLOW -Wno-write-strings -lstdc++fs -lm -O3 -std=c++17 -I../../source/shared_code -I../../include -I../../source/molflow_code -I../../include/SDL ../../source/shared_code/Vector.cpp 
cd ../../linux_build
