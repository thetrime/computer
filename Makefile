# export LD_LIBRARY_PATH=/opt/sphinx/lib:/opt/swi-prolog/lib/swipl-7.7.18/lib/armv7l-linux/
# sudo apt-get install flite1-dev
SPHINX_CFLAGS=-I/opt/sphinx/include/pocketsphinx -I/opt/sphinx/include/sphinxbase -I/opt/sphinx/include
SWI_CFLAGS=-I/opt/swi-prolog/lib/swipl-7.7.18/include
FLITE_CFLAGS=
SPHINX_LDFLAGS=-L/opt/sphinx/lib -lpocketsphinx -lsphinxbase -lsphinxad
SWI_LDFLAGS=-L/opt/swi-prolog/lib/swipl-7.7.18/lib/armv7l-linux -lswipl
FLITE_LDFLAGS=-L/usr/lib/arm-linux-gnueabihf -lflite_cmu_us_awb -lflite_usenglish -lflite_cmulex -lflite -lm -lasound

all:	computer.name computer.lm computer.dic sphinx.so 

sphinx.so:	sphinx.o
	gcc $(SPHINX_LDFLAGS) $(SWI_LDFLAGS) -shared sphinx.o -o sphinx.so

sphinx.o:	sphinx.c
	gcc -c $(SPHINX_CFLAGS) $(SWI_CFLAGS) -g sphinx.c -o sphinx.o

flite.so:	flite.o
	gcc flite.o $(FLITE_LDFLAGS) $(SWI_LDFLAGS) -shared -o flite.so

flite.o:	flite.c
	gcc -c $(FLITE_CFLAGS) $(SWI_CFLAGS) flite.c -o flite.o

computer.name computer.lm computer.dic:	corpus.txt
	curl `curl "http://www.speech.cs.cmu.edu/cgi-bin/tools/lmtool/run" -F "corpus=@corpus.txt" -F "formtype=simple" -sL -H "Content-Type: multipart/form-data" | grep "TAR" | grep "product" | sed -e 's@.*href="\\(.*\\)".*@\1@g'` | tar -xzv
	mv *.lm computer.lm
	mv *.dic computer.dic
	echo "COMPUTER" > computer.name
	rm -f TAR*.tgz
	rm -f *.log_pronounce
	rm -f *.sent
	rm -f *.vocab
