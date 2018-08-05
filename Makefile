# export LD_LIBRARY_PATH=/opt/sphinx/lib:/opt/swi-prolog/lib/swipl-7.7.18/lib/armv7l-linux/

CFLAGS=-I/opt/sphinx/include/pocketsphinx -I/opt/sphinx/include/sphinxbase -I/opt/sphinx/include -I/opt/swi-prolog/lib/swipl-7.7.18/include -g
LDFLAGS=-L/opt/sphinx/lib -L/opt/swi-prolog/lib/swipl-7.7.18/lib/armv7l-linux -lpocketsphinx -lsphinxbase -lswipl

sphinx.so:	sphinx.o
	gcc $(LDFLAGS) -shared sphinx.o -o sphinx.so

sphinx.o:	sphinx.c
	gcc -c $(CFLAGS) sphinx.c -o sphinx.o

