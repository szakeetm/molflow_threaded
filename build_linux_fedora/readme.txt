To compile (cpp -> o) run ./compile_linux_fedora.sh (will write files in intermediary/linux_fedora)
To link (o -> binary) run ./link_linux_fedora.sh (will write files in bin/linux_fedora)
To clean o files: ./clean_linux_fedora.sh

yum packages to compile:

libpng-devel
gtk3-devel

(yum-config-manager --add-repo EPEL)

SDL2-devel
gsl-devel
libcurl-devel

Tested using Centos 7