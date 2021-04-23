/*
 * Filename: prga-hw08-main.c
 * Date:     2019/12/25 22:57
 * Author:   Jan Faigl
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
 
#include <termios.h> 
#include <unistd.h>  // for STDIN_FILENO
 
#include <pthread.h>

#include "prg_io_nonblock.h"
 
void call_termios(int reset);

 
int main(int argc, char *argv[]){
  
   // TODO open pipe

   call_termios(0); // store terminal settings

   // TODO create and initialize threads

   call_termios(1); // restore terminal settings
   return EXIT_SUCCESS;
}

// - function -----------------------------------------------------------------
void call_termios(int reset){
   static struct termios tio, tioOld;
   tcgetattr(STDIN_FILENO, &tio);
   if (reset) {
      tcsetattr(STDIN_FILENO, TCSANOW, &tioOld);
   } else {
      tioOld = tio; //backup 
      cfmakeraw(&tio);
      tio.c_oflag |= OPOST;
      tcsetattr(STDIN_FILENO, TCSANOW, &tio);
   }
}

/* end of prga-hw08-main.c */
