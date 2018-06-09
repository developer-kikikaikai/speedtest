TARGET=mallocspeed
SRC=$(shell find . -maxdepth 1 -name "*.c")
#CFLAG=-g -O0 
CFLAG=-O2
LIBS=../../lib/.libs/libtimelog.a own/libown_malloc.a hhs/libhhs_malloc.a -lpthread 
CFLAG+=-DUSE_TCMALLOC 
LIBS+=-Xlinker -rpath -Xlinker /usr/local/lib/  -ltcmalloc
INC=-I ../../include -I /usr/local/include/ -Iown -Ihhs
OBJ=$(SRC:%.c=%.o)

all: sub $(OBJ)
	gcc $(CFLAG) $(OBJ) $(LIBS) -o $(TARGET)

%.o: %.c
	gcc $(CFLAG) $(INC) -o $@ -c $<

sub:
	make -C hhs
	make -C own
	make -C libtest 

clean:
	make -C hhs clean
	make -C own clean
	make -C libtest clean
	rm -f $(OBJ) $(TARGET);
