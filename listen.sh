#!/bin/bash

source env.sh
MODEL=$1

pocketsphinx_continuous -dict $MODEL.dic \
                        -lm $MODEL.lm \
                        -inmic 1 \
			-adcdev plughw:1

#			-keyphrase "computer" -kws_threshold 1e-30 \
