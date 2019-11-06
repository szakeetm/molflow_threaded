To compile (cpp -> o) run ./compile_linux_opensuse.sh (will write files in intermediary/linux_opensuse)
To link (o -> binary) run ./link_linux_opensuse.sh (will write files in bin/linux_opensuse)
To clean o files: ./clean_linux_opensuse.sh

zypper packages needed to compile:

g++-8
libpng16-devel
libSDL2-devel
gsl-devel
libcurl-devel

and to link:

gtk3-devel
cblas-devel
(atlascpp-devel)
(blas-devel)

Tested using OpenSUSE 15.1 in a virtual machine
