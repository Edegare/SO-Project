CC = gcc
CFLAGS = -Wall -g -Iinclude `pkg-config --cflags --libs glib-2.0`
LDFLAGS = `pkg-config --libs glib-2.0`

all: folders orchestrator client

orchestrator: bin/orchestrator

client: bin/client

folders:
	@mkdir -p src include obj bin tmp

bin/orchestrator: obj/orchestrator.o obj/aux.o obj/parser.o
	$(CC) $^ -o $@ $(LDFLAGS)

bin/client: obj/client.o
	$(CC) $(LDFLAGS) $^ -o $@ 

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f obj/* tmp/* bin/* src/fifo*
