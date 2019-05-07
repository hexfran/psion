CC=gcc
CFLAGS=-std=c99 -Wall -Werror -Wextra -pedantic -I. -I../libs/sds -I../libs/trie -I../libs/picohttpparser -I../libs/cJSON -O2 -D_FORTIFY_SOURCE=2 -fpie -Wl,-pie -fpic -fstack-clash-protection -fstack-protector-all -fstack-protector-strong -fcf-protection -Werror=format-security -Werror=implicit-function-declaration -Wl,-z,defs -Wl,-z,now -Wl,-z,relro 
DEBUG_CFLAGS=-g -g3 -ggdb -ggdb3 -grecord-gcc-switches
NEEDED_C_FILES = ../libs/sds/sds.c ../libs/trie/trie.c ../libs/picohttpparser/picohttpparser.c ../libs/cJSON/cJSON.c

psion: $(NEEDED_C_FILES) $(wildcard *.c)
	$(CC) $^ $(CFLAGS) -o ../build/$@ 

psion-debug: $(NEEDED_C_FILES) $(wildcard *.c)
	$(CC) $^ $(DEBUG_CFLAGS) $(CFLAGS) -o ../build/$@ 

clean:
	rm -f ../build/*
