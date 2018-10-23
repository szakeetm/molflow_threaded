To compile (cpp -> o) run ./compile_linux.sh (will write files in intermediary/linux)
To link (o -> binary) run ./link_linux.sh (will write files in bin/linux)
To clean o files: ./clean_linux.sh

Packages to compile:

g++-8
libpng-dev

And to link:

gsl-bin
pkg-config
gtk+-3.0
libgsl-dev
libcurl4-gnutls-dev
libatlas-base-dev
libsdl2-dev

Tested using Ubuntu 18.04, both in a virtual machine and natively with a Linux shell under Windows 10