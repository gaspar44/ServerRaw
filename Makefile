GCC=gcc
#GCCFLAGS= -g -std=c99 #-Wall -Werror

%.o: %.c
	$(GCC) -c $(GCCFLAGS) $<

all: main.o
	$(GCC) -o main main.o
	
clean:
	rm *.o main