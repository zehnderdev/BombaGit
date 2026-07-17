CC = gcc
LDLIBS = -lcrypto
CFLAGS = -Wall -Wextra -Werror -std=c17
DST  = bgit

SRC = src/main.c src/sha256.c #need both

# $@ left and $^ right of ":"
$(DST): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean

clean:
	rm -f $(DST) *.o

