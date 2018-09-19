# export LD_LIBRARY_PATH=/opt/sphinx/lib:/opt/swi-prolog/lib/swipl-7.7.18/lib/armv7l-linux/
# sudo apt-get install flite1-dev libfftw3-dev

SPHINX_CFLAGS=-I/opt/sphinx/include/pocketsphinx -I/opt/sphinx/include/sphinxbase -I/opt/sphinx/include
SWI_CFLAGS=`PKG_CONFIG_PATH=/opt/swi-prolog/lib/pkgconfig/ pkg-config swipl --cflags`
FLITE_CFLAGS=`pkg-config fftw3 --cflags`
FFTW_CFLAGS=-I/opt/fftw/include
TENSORFLOW_CFLAGS=-I/opt/tensorflow/include
SPHINX_LDFLAGS=-L/opt/sphinx/lib -lpocketsphinx -lsphinxbase -lsphinxad
SWI_LDFLAGS=`PKG_CONFIG_PATH=/opt/swi-prolog/lib/pkgconfig/ pkg-config swipl --libs`
FLITE_LDFLAGS=-L/usr/lib/arm-linux-gnueabihf -lflite_cmu_us_awb -lflite_usenglish -lflite_cmulex -lflite -lm -lasound
TENSORFLOW_LDFLAGS=-L/opt/tensorflow/lib -ltensorflow
FFTW_LDFLAGS=`pkg-config fftw3 --libs`

all:	default.lm default.dic weather.lm weather.dic sphinx.so 

sphinx.so:	sphinx.o
	gcc $(SPHINX_LDFLAGS) $(SWI_LDFLAGS) -shared sphinx.o -o sphinx.so

sphinx.o:	sphinx.c
	gcc -c $(SPHINX_CFLAGS) $(SWI_CFLAGS) -g sphinx.c -o sphinx.o

flite.so:	flite.o
	gcc flite.o $(FLITE_LDFLAGS) $(SWI_LDFLAGS) -shared -o flite.so

flite.o:	flite.c
	gcc -c $(FLITE_CFLAGS) $(SWI_CFLAGS) flite.c -o flite.o

ibuprofen.o:	ibuprofen.c
	gcc -g $(TENSORFLOW_CFLAGS) -c $< -o $@

libuprofen.so: libuprofen.o holmes.o ibuprofen.o sphinx.so
	gcc libuprofen.o holmes.o ibuprofen.o $(SPHINX_LDFLAGS) $(TENSORFLOW_LDFLAGS) $(FFTW_LDFLAGS) $(SWI_LDFLAGS) -L. -l:sphinx.so -shared -o $@

libuprofen.o: libuprofen.c
	gcc -c libuprofen.c $(SWI_CFLAGS) $(FFTW_CFLAGS) $(TENSORFLOW_CFLAGS) -o $@

holmes.o: holmes.c
	gcc -g $(FFTW_CFLAGS) $(TENSORFLOW_CFLAGS) -c $< -o $@

%.lm %.dic: %.txt
	$(eval ID=`curl "http://www.speech.cs.cmu.edu/cgi-bin/tools/lmtool/run" -F "corpus=@$<" -F "formtype=simple" -sL -H "Content-Type: multipart/form-data" | grep "is the compressed" | sed -e 's@.*\(http://.*\)/TAR\(.*\).tgz".*@\1/\2@g'`)
	curl -s "$(ID).lm" -o $(basename $@).lm
	curl -s "$(ID).dic" -o $(basename $@).dic
#	(curl -s "$(ID).dic" -o - ; cat master_dictionary) | sort -u -o master_dictionary
