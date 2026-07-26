CC = gcc
CFLAGS = -Wall -Wextra -O2
INCLUDE = include/ 

TARGET = bin/demo

SRC = src/game/*.c src/engine/*.c

$(TARGET):$(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -I $(INCLUDE) 

clean:
	-rm -f $(TARGET)
	
