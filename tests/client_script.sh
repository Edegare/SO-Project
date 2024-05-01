#!/bin/bash

# Array with commands
commands=(
    "execute 500 -u 'ls -l'"
    "execute 2000 -p 'grep apple tests/tests.txt | wc -l'"
    "execute 2000 -p 'grep hi tests/tests.txt | wc -l'"
    "execute 1000 -p 'ls -l | grep tests'"
    "execute 2000 -p 'ls -l | grep tests | wc -l'"
    "execute 1500 -u 'grep apple tests/tests.txt'"
    "execute 2500 -p 'ls -l | grep tests | wc -w'"
    "execute 500 -u 'ls -l'"
    "execute 2000 -p 'grep apple tests/tests.txt | wc -l'"
    "execute 2000 -p 'grep hi tests/tests.txt | wc -l'"
    "execute 1000 -p 'ls -l | grep tests'"
    "execute 2000 -p 'ls -l | grep tests | wc -l'"
    "execute 1500 -u 'grep apple tests/tests.txt'"
    "execute 2500 -p 'ls -l | grep tests | wc -w'"
    "execute 500 -u 'ls -l'"
    "execute 2000 -p 'grep apple tests/tests.txt | wc -l'"
    "execute 2000 -p 'grep hi tests/tests.txt | wc -l'"
    "execute 1000 -p 'ls -l | grep tests'"
    "execute 2000 -p 'ls -l | grep tests | wc -l'"
    "execute 1500 -u 'grep apple tests/tests.txt'"
    "execute 2500 -p 'ls -l | grep tests | wc -w'"
)

# Loop commands to create clients
for cmd in "${commands[@]}"; do
    eval ./bin/client $cmd
done
