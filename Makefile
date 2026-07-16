CC = gcc
CFLAGS = -lcrypto

DST  = bgit

SRC = src/main.c src/sha256.c #need both

# $@ left and $^ right of ":"
$(DST): $(SRC)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(DST) *.o

