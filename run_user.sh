#!/bin/bash

cd "/mnt/c/Users/ARNAB GHOSH/mytop" || exit 1

gcc top_simple.c -o mytop

if [ $? -eq 0 ]; then
    ./mytop
fi break; done