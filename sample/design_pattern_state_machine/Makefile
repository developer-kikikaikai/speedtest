TARGET=specStateMachine
SRC=$(shell find . -maxdepth 1 -name *.c)
CFLAG=-g -O0 
LIBS=../../lib/.libs/libtimelog.a -lrt -lpthread -lstate_machine -levent_threadpool -L/usr/local/lib -Xlinker -rpath -Xlinker /usr/local/lib
INC=-I ../../include -I/usr/local/include
OBJ=$(SRC:%.c=%.o)

all: $(OBJ)
	gcc $(CFLAG) $(INC) $(OBJ) $(LIBS) -o $(TARGET)

%.o: %.c
	gcc $(INC) $(LIBS) -o $@ -c $<

clean:
	rm -f $(OBJ) $(TARGET);
