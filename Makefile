CC = gcc
CFLAGS = -Wall -Wextra
INCLUDE = include/ 

TARGET = bin/rogue

SRC = src/game/*.c src/engine/*.c

$(TARGET):$(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -I $(INCLUDE) 

clean:
	-rm -f $(TARGET)
	
