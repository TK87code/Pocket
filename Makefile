CC = gcc
INCLUDE = include/ 

TARGET = bin/demo

SRC = src/game/*.c src/engine/*.c

ifeq ($(DEBUG), 1)
	CFLAGS = -Wall -Wextra -g -O0 -DPKT_DEBUG
else
	CFLAGS = -Wall -Wextra -O2
endif
	
$(TARGET):$(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -I $(INCLUDE) 

clean:
	-rm -f $(TARGET)
	
