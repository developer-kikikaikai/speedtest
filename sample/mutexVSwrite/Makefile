TARGET=mutexVSwrite
SRC=$(shell find . -maxdepth 1 -name *.c)
CFLAG=-g -O0 
LIBS=../../lib/.libs/libtimelog.a -lrt -lpthread
INC=-I ../../include
OBJ=$(SRC:%.c=%.o)

all: $(OBJ)
	gcc $(CFLAG) $(OBJ) $(LIBS) -o $(TARGET)

%.o: %.c
	gcc $(INC) -o $@ -c $<

clean:
	rm -f $(OBJ) $(TARGET);
