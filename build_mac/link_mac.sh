cd ../bin/mac
g++-8 -o molflow ../../precomp_obj/mac/*.o ../../intermediary/mac/*.o -framework OpenGL -L/usr/local/lib -lSDL2 -lpng -lgsl -lcurl  -lstdc++ -lstdc++fs -framework AppKit
cd ../../build_mac