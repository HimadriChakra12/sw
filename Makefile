CC = cc
CFLAGS = -std=c99 -pedantic -Wall -Wextra \
         -I/usr/include/dbus-1.0 \
         -I/usr/lib/dbus-1.0/include \
         -I/usr/lib64/dbus-1.0/include

LDFLAGS = -s -Os -ldbus-1

TARGET = sw
OBJ = sw.o
SRC = sw.c

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
