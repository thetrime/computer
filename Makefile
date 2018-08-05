# export LD_LIBRARY_PATH=/opt/sphinx/lib:/opt/swi-prolog/lib/swipl-7.7.18/lib/armv7l-linux/

CFLAGS=-I/opt/sphinx/include/pocketsphinx -I/opt/sphinx/include/sphinxbase -I/opt/sphinx/include -I/opt/swi-prolog/lib/swipl-7.7.18/include -g
LDFLAGS=-L/opt/sphinx/lib -L/opt/swi-prolog/lib/swipl-7.7.18/lib/armv7l-linux -lpocketsphinx -lsphinxbase -lswipl -lsphinxad

all:	computer.name computer.lm computer.dic sphinx.so 

sphinx.so:	sphinx.o
	gcc $(LDFLAGS) -shared sphinx.o -o sphinx.so

sphinx.o:	sphinx.c
	gcc -c $(CFLAGS) sphinx.c -o sphinx.o

computer.name computer.lm computer.dic:	corpus.txt
	curl `curl "http://www.speech.cs.cmu.edu/cgi-bin/tools/lmtool/run" -F "corpus=@corpus.txt" -F "formtype=simple" -sL -H "Content-Type: multipart/form-data" | grep "TAR" | grep "product" | sed -e 's@.*href="\\(.*\\)".*@\1@g'` | tar -xzv
	mv *.lm computer.lm
	mv *.dic computer.dic
	echo "COMPUTER" > computer.name
	rm -f TAR*.tgz
	rm -f *.log_pronounce
	rm -f *.sent
	rm -f *.vocab
