cd ../bin/mac
g++-9 ../../source/shared_code/compress/compress.cpp ../../source/shared_code/File.cpp -o compress -I../../source/shared_code -std=c++17 -lstdc++fs -Wno-write-strings
cd ../../build_mac