TARGET=test
SRC=$(shell find . -maxdepth 1 -name "*.c")
CFLAG=-g -O0 
LIBS= ../hhs/libhhs_malloc.a ../own/libown_malloc.a -lpthread
LIBS+=../../../lib/.libs/libtimelog.a 
INC=-I ../../include -I ../../../include -I ../hhs/ -I../own/
OBJ=$(SRC:%.c=%.o)

all: $(OBJ)
	gcc $(CFLAG) $(OBJ) $(LIBS) -o $(TARGET)

%.o: %.c
	gcc $(INC) $(CFLAG) $(LIBS) -o $@ -c $<

clean:
	rm -f $(OBJ) $(TARGET);
