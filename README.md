**Martin Holoubek**

**_Interactive applications with multi-threads communication_**

_Recommended compilation:_

cc -c -Wall -Werror -std=gnu99 -g prga-hw08-main.c -o prga-hw08-main.o

cc prga-hw08-main.o prg_io_nonblock.o -pthread -o prga-hw08-main

Before the program executing, make sure your in and out pipes are opened. 
Program is executed by running the binary prga-hw08-main file with the module file in another terminal or device

By default, the pipes are specified with names "/tmp/prga-hw08.out" and "/tmp/prga-hw08.in".
If you want to use your own ones, put them as arguments after the file execution.
As first pipe, specify the one that's used as the module output pipe. After that, use the module input pipe.

Example of program configuration.

$ mkfifo my_fifo_in
$ mkfifo my_fifo_out
$ ./prga-hw08-module my_fifo_out my_fifo_in
$ ./prga-hw08-main my_fifo_out my_fifo_in

Now you can use the implemened commands.


's' - Led On
'e' - Led Off
'h' - Saying 'Hello'
'b' - Saying 'Bey' and quiting the program
'1'-'5' - Setting the period of led flashing


**Have fun :))**



