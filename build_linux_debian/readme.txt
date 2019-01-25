To compile (cpp -> o) run ./compile_linux_debian.sh (will write files in intermediary/linux_debian)
To link (o -> binary) run ./link_linux_debian.sh (will write files in bin/linux_debian)
To clean o files: ./clean_linux_debian.sh

apt packages needed to compile:

g++-8
libpng-dev
libsdl2-dev

and to link:

gsl-bin
pkg-config
gtk+-3.0
libgsl-dev
libcurl4-gnutls-dev
libatlas-base-dev

Tested using Ubuntu 18.04, both in a virtual machine and natively with a Linux shell under Windows 10
