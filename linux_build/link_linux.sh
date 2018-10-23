cd ../bin/linux
g++-8 -o molflow ../../precomp_obj/linux/*.o ../../intermediary/linux/*.o `pkg-config --libs gtk+-3.0` -lGL -L/usr/local/lib  -lSDL2 -lpng -lgsl -lcurl  -g -pthread -lX11 -lcblas -lstdc++fs
cd ../../linux_build
