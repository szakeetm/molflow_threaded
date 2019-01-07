# MolFlow+
A Monte Carlo simulator for Ultra High Vacuum systems

**Authors:** Marton ADY, Roberto KERSEVAN, Jean-Luc PONS  
**Website:** https://cern.ch/molflow  
**Copyright:** CERN (2018)  
**License:** GNU GPLv2 or later

<img src="https://molflow.web.cern.ch/sites/molflow.web.cern.ch/files/pictures/2018-10-09%2016_14_20-PowerPoint%20Slide%20Show%20%20-%20%20Presentation1.png" alt="Molflow image" width="800"/>

# Building
## Windows
* Open *molflow_win.sln* in the *Visual_Studio* folder, and use Visual Studio to build the solution.  
Tested with Visual Studio Community 2017.
## Linux (Debian)
* Scripts in the *build_linux* folder  
* Compile source files with *compile_linux.sh* and link with *link_linux.sh*. The resulting binaries are written in *bin/linux_debian*  
Makefile coming soon.  
See readme.txt  for required packages to build.  
## Linux (Fedora-based, like Cent OS)
* You have to clone and build GCC 8.2 first.
* The compile scripts expect a folder named gcc_8.2.0_compiled_fedora adjacent to this repository.

## Mac
* Use Homebrew to install build tools, like g++-8, the SDL2 library, libpng, gsl, curl  
* The compile and link scripts are in the *build_mac* folder.  The resulting binaries are written in *bin/mac* 
* Compile source files with *compile_linux.sh* and link with *link_linux.sh*. The resulting binaries are written in *bin/linux_fedora* 
 
# Running
## Windows
Use the shortcut (that changes the working directory and launches *molflow.exe*) in *bin\win\release*
## Linux (Debian)
* Install dependencies, like *libsdl2-2.0*, *gsl-bin*, *libatlas-base-dev*  
* In the *bin/linux* folder, make *molflow*, *7za* and *compress* executable
* Run *molflow*  
[Detailed instructions here](https://molflow.web.cern.ch/node/296)
## Linux (Fedora-based, like Cent OS)
* Make *launch_molflow.sh* executable
* Run *launch_molflow.sh* (It adds the lib folder to the library search path and launches molflow)
[Detailed instructions here](https://molflow.web.cern.ch/node/302)
## Mac
* Use Homebrew to install dependencies, like *sdl2*, *libpng*, *gsl*, *gcc*  
* In the *bin/mac* folder, make *molflow*, *7za* and *compress* executable
* Run *molflow*  

[Detailed instructions here](https://molflow.web.cern.ch/node/294)

# Repository snapshots
Commits are constantly pushed to this primary repo, and some of them might break - temporarily - the build scripts. If you want to fork Molflow, it is recommended that you download a [snapshot](https://molflow.web.cern.ch/content/developers) of a guaranteed-to-work state. Usually these snapshots are made at every public release of Molflow.
