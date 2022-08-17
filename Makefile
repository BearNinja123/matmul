CC = gcc
CFLAGS = -O3 -funroll-all-loops -march=native
CPP_FLAGS=
LIBS = -lm

ifndef NO_OMP
	CPP_FLAGS+=-fopenmp
endif
ifndef NO_BLAS
	LIBS += -lblas
	CPP_FLAGS += -DHAVE_BLAS
endif


NPROCS = $(shell grep -c 'processor' /proc/cpuinfo)
MAKEFLAGS += -j$(NPROCS)


all: mm_object arr_object main.c
	$(CC) -o mm arr.o mm.o main.c $(CFLAGS) $(CPP_FLAGS) $(LIBS)

arr_object: arr.c
	$(CC) -c arr.c $(CFLAGS) $(CPP_FLAGS) $(LIBS)

mm_object: mm.c
	$(CC) -c mm.c $(CFLAGS) $(CPP_FLAGS) $(LIBS)
