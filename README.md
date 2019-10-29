# CustomShell
Custom shell written for Linux as a part of college class.

## Supported: 
&,
Single pipe |,
Zombie handler

### Builtin commands:
help - list of commands
jobs - lists all jobs created in lsh
kill id - kills job identifed by id
exit - exit lsh

## Installation
download lsh.c
compile lsh.c by using gcc -o lsh lsh.c
run by ./lsh

## Usage
            lsh> sleep5&,
            lsh> jobs
            lsh> ls -l | sort
            lsh> exit
