CFLAGS=-I/opt/sphinx/include/pocketsphinx -I/opt/sphinx/include/sphinxbase -I/opt/sphinx/include -I/opt/swi-prolog/lib/swipl-7.7.18/include
LDFLAGS=-L/opt/sphinx/lib -L/opt/swi-prolog/lib/swipl-7.7.18/lib/armv7l-linux -lpocketsphinx -lsphinxbase -lswipl

main:	main.c
	gcc $(CFLAGS) $(LDFLAGS) main.c -o main 
