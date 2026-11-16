#!/bin/bash

resize -s 22 100

gcc top_simple.c -o mytop

if [ $? -eq 0 ]; then
    ./mytop
fi