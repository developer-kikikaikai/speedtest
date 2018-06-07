TARGET=libown_malloc
RMTARGET=$(TARGET).*
SRC=$(shell find . -maxdepth 1 -name "*.c")
CFLAG=-O2 -shared -fPIC
#CFLAG=-O0 -g -shared -fPIC
INCLUDE=-I ../include
LIBS=-lpthread
all:
	gcc $(CFLAG) $(INCLUDE) $(OBJ) $(LIBS) -o $(TARGET).a $(SRC)
	gcc $(CFLAG) $(INCLUDE) $(OBJ) $(LIBS) -o $(TARGET).so $(SRC)

clean:
	rm -f $(OBJ) $(RMTARGET);
