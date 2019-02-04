# Makefile
CC=gcc
CFLAGS=
LDFLAGS=-lncurses -lpthread -lmenu -lm
LCFLAGS=-lncurses -lmenu
LEFLAGS=-lncurses -lpanel

crash_test : crash_test.o
	$(CC) $(CFLAGS) crash_test.o -o crash_test $(LCFLAGS)

crash_test.o: crash_test.c
	$(CC) $(CFLAGS) -c crash_test.c $(LCFLAGS)

test_panel : test_panel.o
	$(CC) $(CFLAGS) test_panel.o -o test_panel $(LEFLAGS)

test_panel.o: test_panel.c
	$(CC) $(CFLAGS) -c test_panel.c $(LEFLAGS)

client: client.o config.o
	$(CC) $(CFLAGS) client.o config.o -o client $(LDFLAGS)

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

serveur: serveur.o config.o
	$(CC) $(CFLAGS) serveur.o config.o -o serveur $(LDFLAGS)

serveur.o: serveur.c
	$(CC) $(CFLAGS) -c serveur.c

config.o: config.h config.c
	$(CC) $(CFLAGS) -c config.c

clean:
	rm -rf *.o