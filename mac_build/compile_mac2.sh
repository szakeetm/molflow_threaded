cd ../intermediary/mac
g++-8 -c -std=c++17 -Wno-write-strings -DMOLFLOW -O3 -I/usr/local/include -I../../source/molflow_code -I../../source/shared_code -I../../include -I../../include/SDL ../../source/molflow_code/SubProcessFacet.cpp
cd ../../mac_build