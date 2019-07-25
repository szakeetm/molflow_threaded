cd ../bin/linux
g++-8 -o molflow ../../precomp_obj/linux_debian/*.o ../../intermediary/linux/*.o `pkg-config --libs gtk+-3.0` -lGL -lSDL2 -lpng -lgsl -lcurl -g -pthread -lX11  -lgslcblas  -lstdc++fs
# -L/usr/local/lib
cd ../../build_linux_opensuse
