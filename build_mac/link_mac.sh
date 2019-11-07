mkdir -p ../bin/mac
cd ../bin/mac
g++-9 -o molflow -L/usr/local/lib ../../precomp_obj/mac/*.o ../../intermediary/mac/*.o -framework OpenGL -lSDL2 -lpng -lgsl -lcurl -lstdc++ -lstdc++fs -framework AppKit
cd ../../build_mac
