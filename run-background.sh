#!/bin/bash

make && nohup /opt/swi-prolog/bin/swipl -f main.pl -t halt -g computer &
