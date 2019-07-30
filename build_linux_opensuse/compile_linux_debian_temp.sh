cd ../intermediary/linux
g++-8 -c -DMOLFLOW -std=c++17 -Wno-write-strings -Wno-unused-result -lstdc++fs -lm -O3  -I../../source/shared_code -I../../include -I../../source/molflow_code -I../../include/SDL ../../source/molflow_code/MolflowWorker.cpp
cd ../../build_linux_opensuse
