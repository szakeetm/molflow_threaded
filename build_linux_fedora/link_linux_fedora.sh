cd ../bin/linux_fedora
$GCC8DIR/bin/g++ -o molflow ../../precomp_obj/linux_fedora/*.o ../../intermediary/linux_fedora/*.o $GCC8DIR/lib64/libstdc++fs.a `pkg-config --libs gtk+-3.0`  -lSDL2 -pthread -lcurl -lgsl -lpng -lGL
# -L/usr/lib64/atlas  -ltatlas -lX11 -g  -L/usr/local/lib
cd ../../build_linux_fedora
