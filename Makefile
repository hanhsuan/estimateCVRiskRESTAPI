CFLAGS = -g -O2 -I/usr/share/R/include/ -I./inc/
LFLAGS = -lR -ljson-c -levent -lsagui

main: src/es.c main.c
	$(CC) -o $@ $(CFLAGS) $^ $(LFLAGS) 

clean:
	rm -f main
