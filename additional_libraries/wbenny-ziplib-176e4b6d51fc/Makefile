# Final names of binaries
EXECUTABLE = Bin/zipsample
SO_LIBRARY = Bin/libzip.so
ST_LIBRARY = Bin/libzip.a

# C & C++ compiler
#CC       = gcc-8
#CXX      = g++-8
CC        = clang
CXX       = clang++
CFLAGS    = -Wno-enum-conversion -O3
CXXFLAGS  = -std=c++11 -stdlib=libc++ -O3
#compile with libc++ when using clang in other projects

# Linker flags
LDFLAGS   = -pthread

# Sources of external libraries
SRC_ZLIB  = $(wildcard Source/ZipLib/extlibs/zlib/*.c)
SRC_LZMA  = $(wildcard Source/ZipLib/extlibs/lzma/unix/*.c)
SRC_BZIP2 = $(wildcard Source/ZipLib/extlibs/bzip2/*.c)

# ZipLib sources
SRC = \
		$(wildcard Source/ZipLib/*.cpp)        \
		$(wildcard Source/ZipLib/detail/*.cpp)

# Object files			
OBJS = \
		$(SRC:.cpp=.o)	   \
		$(SRC_ZLIB:.c=.o)  \
		$(SRC_LZMA:.c=.o)  \
		$(SRC_BZIP2:.c=.o)

# Rules
all: $(EXECUTABLE)  $(ST_LIBRARY)
	
sample: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) Source/Sample/Main.cpp -o $@ $^

$(SO_LIBRARY): $(OBJS)
	$(CXX) $(LDFLAGS) -shared -o $@ $^

$(ST_LIBRARY): $(OBJS)
	ar ru $@ $^
	ranlib $@
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf `find Source -name '*.o'` ziplib.tar.gz Bin/*.zip Bin/out* $(EXECUTABLE) $(SO_LIBRARY) $(ST_LIBRARY)

tarball:
	tar -zcvf ziplib.tar.gz *
	
