TARGET=sample
SRC=$(shell find . -maxdepth 1 -name *.c)
CFLAG=-g -O0 
#CFLAG+=-DTHREAD_SAFE
LIBS=../../lib/.libs/libtimelog.a -lpthread -lrt
INC=-I ../../include
OBJ=$(SRC:%.c=%.o)

all: $(OBJ)
	gcc $(OBJ) $(LIBS) -o $(TARGET)

%.o: %.c
	gcc $(CFLAG) $(INC) -o $@ -c $<

clean:
	rm -f $(OBJ) $(TARGET);
