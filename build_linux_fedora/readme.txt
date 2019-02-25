The scripts below expect g++ 8.2.0 to be ***installed*** (bin, lib, lib64 and include folders) under $GCC82DIR (or will look for binaries in /usr/bin)
To build it:

1) Download gcc source (wget) and extract (tar xvf)
2) Download prerequisites (contribute/download_prerequisites)
3) make build dir and cd to it
4) ../configure --prefix=$GCC82DIR --enable-languages=c,c++ --disable-multilib
5) make
6) make install

(They call $GCC82DIR/bin/g++ for compiling and
$GCC82DIR/lib64/libstdc++fs.a for linking)

To compile (cpp -> o) run ./compile_linux_fedora.sh (will write files in intermediary/linux_fedora)
To link (o -> binary) run ./link_linux_fedora.sh (will write files in bin/linux_fedora)
To clean o files: ./clean_linux_fedora.sh

yum packages to install for compiling:

libpng-devel
gtk3-devel

Packages below require EPEL repository:
(yum-config-manager --add-repo EPEL)
OR
yum -y install epel-release
THEN
Refresh repo by typing the following command: yum repolist

SDL2-devel
gsl-devel
libcurl-devel
atlas-devel

Tested using Centos 7
