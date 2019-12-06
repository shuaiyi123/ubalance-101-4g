
CC := arm-linux-gnueabihf-gcc
#CC  =gcc
#INCLUDE= .
#LIBS   =
MACRO_DEFINE  = -DLOG
CFLAGS  = -Wall -g
OBJS  =n720.o sysfs_io.o siec101_2002.o siec101_2002_recv.o siec101_2002_trans.o terminal_io.o main.o
TARGET=main.exe

all: $(TARGET)

$(TARGET):$(OBJS) 
	$(CC) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(MACRO_DEFINE)

.PHONY: all clean cleanobject
clean:
	rm -rf $(TARGET) main *.o
cleanobject:
	rm -rf *.o
