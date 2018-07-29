# export LD_LIBRARY_PATH=/opt/sphinx/lib:/opt/swi-prolog/lib/swipl-7.7.18/lib/armv7l-linux/

CFLAGS=-I/opt/sphinx/include/pocketsphinx -I/opt/sphinx/include/sphinxbase -I/opt/sphinx/include -I/opt/swi-prolog/lib/swipl-7.7.18/include
LDFLAGS=-L/opt/sphinx/lib -L/opt/swi-prolog/lib/swipl-7.7.18/lib/armv7l-linux -lpocketsphinx -lsphinxbase -lswipl

computer:	main.o prolog.o
	gcc $(LDFLAGS) main.o prolog.o -o computer

main.o:		main.c
	gcc -c $(CFLAGS) main.c -o main.o

prolog.o:	prolog.c
	gcc -c $(CFLAGS) prolog.c -o prolog.o
