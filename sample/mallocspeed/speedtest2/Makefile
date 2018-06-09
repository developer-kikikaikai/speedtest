SRC=$(shell find . -maxdepth 1 -name "*.c")
#CFLAG=-g -O0 
CFLAG=-O2
LIBS=../../../lib/.libs/libtimelog.a ../own/libown_malloc.a ../hhs/libhhs_malloc.a  -lpthread 
CFLAG+=-DUSE_TCMALLOC 
LIBS+=-Xlinker -rpath -Xlinker /usr/local/lib/  -ltcmalloc
INC=-I ../../../include -I /usr/local/include/ -I../own -I../hhs 
OBJ=$(SRC:%.c=%_run)

all: $(OBJ)

%_run: %.c
	gcc $< $(CFLAG) $(INC) $(LIBS) -o $@

clean:
	rm -f $(OBJ) ;
