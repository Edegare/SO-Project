#!/bin/bash

# Array with commands
commands=(
    "execute 10 -u 'ls -l'"
    "execute 15 -p 'grep apple tests/tests.txt | wc -l'"
    "execute 15 -p 'grep hi tests/tests.txt | wc -l'"
    "execute 15 -p 'ls -l | grep tests'"
    "execute 20 -p 'ls -l | grep tests | wc -l'"
    "execute 5 -u 'grep apple tests/tests.txt'"
    "execute 5 -p 'ls -l | grep tests | wc -w'"
    "execute 10 -u 'ls -l'"
    "execute 20 -p 'grep -v ^# /etc/passwd | cut -f7 -d: | uniq | wc -l'"
    "execute 10 -p 'grep apple tests/tests.txt | wc -l'"
    "execute 10 -p 'grep hi tests/tests.txt | wc -l'"
    "execute 10 -p 'ls -l | grep tests'"
    "execute 30 -p 'cat tests/tests.txt | grep hi | wc -l'"
    "execute 20 -p 'ls -l | grep tests | wc -l'"
    "execute 5 -u 'grep apple tests/tests.txt'"
    "execute 10 -p 'ls -l | grep tests | wc -w'"
    "execute 10 -u 'ls -l'"
    "execute 20 -p 'grep apple tests/tests.txt | wc -l'"
    "execute 15 -p 'grep hi tests/tests.txt | wc -l'"
    "execute 15 -p 'ls -l | grep tests'"
    "execute 20 -p 'ls -l | grep tests | wc -l'"
    "execute 5 -u 'grep apple tests/tests.txt'"
    "execute 20 -p 'ls -l | grep tests | wc -w'"
    "execute 50 -p 'grep -v ^# /etc/passwd | cut -f7 -d: | uniq | wc -l'"
    "execute 25 -p 'cat tests/tests.txt | grep hi | wc -l'"
)

# Loop commands to create clients
for cmd in "${commands[@]}"; do
    eval ./bin/client $cmd
done
