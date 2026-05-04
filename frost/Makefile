CXX=g++
CFLAGS=-march=native -std=c++17 -O3 -pthread -maes -msse4.2 -mavx2 -funroll-loops -Wno-ignored-attributes -Wno-unused-result
INCLUDE_PATH=-I. -I../ -I/home/$(USER)/FROST/include -I../include -I/usr/local/include

NTL_LIB=/home/$(USER)/FROST/lib/libntl.a
GMP_LIB=/home/$(USER)/FROST/lib/libgmp.a 
OPENSSL_LIB=-L/usr/local/lib 

DEPS=$(NTL_LIB) $(GMP_LIB) $(OPENSSL_LIB)
LIBS=-lm -lcrypto -lstdc++fs 

.PHONY: all clean

all:
	$(CXX) $(CFLAGS) $(INCLUDE_PATH) *.cpp -o main $(LD_FLAGS) $(DEPS) $(LIBS)

clean:
	rm -f main

