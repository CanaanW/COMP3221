#
# Simple makefile for coursework 3.
# Should work on both a School machine once CUDA has been loaded (e.g. moodule avail cuda/...),
# and also on a Mac. Note however that all submissions will be assessed on a School machine.
#
EXE = cwk3
OS = $(shell uname)

ifeq ($(OS), Linux)
	MSG = On a School machine, first load the nvcc compiler using module load cuda
	CC = nvcc -lOpenCL
endif

ifeq ($(OS), Darwin)
	MSG = Mac uses clang with deprecation warnings silenced. May need to change compiler and options depending on your set-up.
	CC = clang -framework OpenCL -DCL_SILENCE_DEPRECATION
endif

all:
	@echo $(MSG)
	@echo
	$(CC) -o $(EXE) cwk3.c



