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

#define PERIOD_STEP 10
#define PERIOD_MAX 2000
#define PERIOD_MIN 10
#define PERIOD_START 100
#define ASCII_1 49

typedef struct {
    int alarm_period;
    int alarm_counter;
    char send_char;
    char received_char;
    bool led;
    bool quit;
    int in_pipe;
    int out_pipe;
} data_t;

pthread_mutex_t mtx;
pthread_cond_t cond;

void call_termios(int reset);

// Vlakna

void* input_thread_kb(void*);
void* input_thread_pipe(void*);
void* output_thread(void*);
void* alarm_thread(void*);

 
int main(int argc, char *argv[]){

    data_t data = { .quit = false,
                  .in_pipe = -1,
                  .out_pipe = -1,
                  .send_char = ' ',
                  .received_char = '?',
                  .led = false,
                  .alarm_period = PERIOD_START,
                  .alarm_counter = 0
    };

    const char *in = argc > 1 ? argv[1] : "/tmp/prga-hw08.out";
    const char *out = argc > 2 ? argv[2] : "/tmp/prga-hw08.in";

    data.in_pipe = io_open_read(in);
    data.out_pipe = io_open_write(out);

    if (data.in_pipe == -1) { // TODO Do shorter
        fprintf(stderr, "Cannot open named pipe port %s\n", in);
        exit(100);
    }
    if (data.out_pipe == -1) {
        fprintf(stderr, "Cannot open named pipe port %s\n", out);
        exit(100);
    }

    call_termios(0); // store terminal settings

    pthread_mutex_init(&mtx, NULL); // Initialize mutex
    pthread_cond_init(&cond, NULL);

    enum { INPUT_KB, INPUT_PIPE, OUTPUT, ALARM, NUM_THREADS };
    const char *thread_names[] = { "Input Keyboard", "Input Pipe", "Output", "Alarm" };
    void* (*thr_functions[])(void*) = { input_thread_kb, input_thread_pipe, output_thread, alarm_thread };
    pthread_t threads[NUM_THREADS];


    for(int i = 0; i < NUM_THREADS; ++i) {
        int r = pthread_create(&threads[i], NULL, thr_functions[i], &data);
        printf("Create thread '%s' %s\r\n", thread_names[i], (r == 0 ? "OK" : "FAIL"));
    }


    int *ex; // For saving returned values
    for(int i = 0; i < NUM_THREADS; ++i){
        printf("Call join to thread '%s'\r\n", thread_names[i]);
        int r = pthread_join(threads[i], (void*)&ex);
        printf("Joining thread '%s' has been %s -- return values %d\r\n", thread_names[i], (r == 0 ? "OK" : "FAIL"), *ex);
    }


    io_close(data.in_pipe);
    io_close(data.out_pipe);

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


void* input_thread_kb(void *arg){
    int c;
    static int ret = 0;
    int period_steps[5] = {50, 100, 200, 500, 1000};
    data_t *data = (data_t*) arg;
    while (!data->quit && (c = getchar()) != EOF) {
        pthread_mutex_lock(&mtx);
        switch(c) {
            case '1': case '2': case '3': case '4': case '5':
                data->alarm_period = period_steps[c - ASCII_1];
            case 's': case 'e': case 'b': case 'h': case 'i':
                if (io_putc(data->out_pipe, c) != 1) {
                    fprintf(stderr, "ERROR: Cannot send command to module, exit program\n");
                    data->quit = true;
                }
                fsync(data->out_pipe);
                data->send_char = c;
                break;
            case 'q':
                fprintf(stderr, "quit\n");
                data->quit = true;
                break;
            default: // discard all other keys
                break;
        }
        pthread_mutex_unlock(&mtx);
    }
    ret = 1;

    return &ret;
}
void* input_thread_pipe(void *arg){
    return NULL;
}
void* output_thread(void *arg){
    return NULL;
}
void* alarm_thread(void *arg){
    return NULL;
}




/* end of prga-hw08-main.c */
