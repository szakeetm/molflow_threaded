cd ../intermediary/linux_fedora
$HOME/gcc-8.2.0/bin/g++ -c -DMOLFLOW -std=c++17 -Wno-write-strings -Wno-unused-result -lstdc++fs -lm -O3  -I../../source/shared_code -I../../include -I/usr/include/sdl -I../../source/molflow_code ../../source/shared_code/*.cpp ../../source/shared_code/GLApp/*.cpp ../../source/shared_code/GLApp/GLChart/*.cpp ../../source/shared_code/Clipper/*.cpp ../../source/shared_code/PugiXML/*.cpp ../../source/shared_code/SDL_SavePNG/*.cpp ../../source/shared_code/TruncatedGaussian/*.cpp ../../source/molflow_code/*.cpp ../../source/shared_code/NativeFileDialog/molflow_wrapper/*.cpp
# -I../../include/SDL
cd ../../build_linux_fedora
