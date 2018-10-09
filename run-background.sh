#!/bin/bash

make && nohup /opt/swi-prolog/bin/swipl -f computer.pl -t halt -g computer &
