CC = gcc
CFLAGS = -Wall -Wextra

TARGET = bin/rogue

SRC = src/game/*.c src/engine/*.c

$(TARGET):$(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) 

clean:
	-rm -f $(TARGET)
	
