# Makefile for ILOC optimizer

CFLAGS=--std=c++11

all:		opt

opt:		opt.o
		clang++ $(CFLAGS) -o opt opt.o

opt.o:		opt.cc
		clang++ $(CFLAGS) -c opt.cc

clean:
		rm *.o
		rm opt

wc:		
		wc -l opt.cc

export:		opt.cc Makefile README
		tar cvf lab3.tar Makefile README opt.cc