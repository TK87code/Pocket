CC = gcc
INCLUDE = include/ 

TARGET = bin/demo

SRC = src/demo/demo.c src/engine/*.c

CFLAGS = -Wall -Wextra -g -O0 -DPKT_DEBUG
	
$(TARGET):$(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -I $(INCLUDE) 

clean:
	-rm -f $(TARGET)
	
