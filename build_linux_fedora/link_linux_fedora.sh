cd ../bin/linux_fedora
PATH=../../../gcc_8.2.0_compiled_fedora/bin:$PATH g++ -o molflow ../../precomp_obj/linux_fedora/*.o ../../intermediary/linux_fedora/*.o ../../../gcc_8.2.0_compiled_fedora/lib64/libstdc++fs.a `pkg-config --libs gtk+-3.0` -lGL -L/usr/local/lib  -L/usr/lib64/atlas -lSDL2 -lpng -lgsl -lcurl  -g -pthread -lX11 -ltatlas
cd ../../build_linux_fedora
