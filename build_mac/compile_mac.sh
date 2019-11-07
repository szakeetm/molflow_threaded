mkdir -p ../intermediary/mac
cd ../intermediary/mac
g++-9 -c -DMOLFLOW -std=c++17 -Wno-write-strings -Wno-unused-result  -I/usr/local/include -I../../source/molflow_code -I../../source/shared_code -I../../include ../../source/shared_code/*.cpp ../../source/shared_code/GLApp/*.cpp ../../source/shared_code/GLApp/GLChart/*.cpp ../../source/shared_code/Clipper/*.cpp ../../source/shared_code/PugiXML/*.cpp ../../source/shared_code/SDL_SavePNG/*.cpp ../../source/shared_code/TruncatedGaussian/*.cpp ../../source/molflow_code/*.cpp ../../source/shared_code/NativeFileDialog/molflow_wrapper/*.cpp -O3
#  -I../../include/SDL
cd ../../build_mac