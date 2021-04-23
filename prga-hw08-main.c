/*
 * Filename: prga-hw08-main.c
 * Date:     2020/04/23 22:57
 * Author:   Martin Holoubek
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
 
#include <termios.h> 
#include <unistd.h>  // for STDIN_FILENO
#include <pthread.h>

#include "prg_io_nonblock.h"

#define READ_TIMEOUT_MS 10

#ifndef PERIOD_SEC
#define PERIOD_SEC 5
#endif


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

void* input_thread_kb(void*);
void* input_thread_pipe(void*);
void* output_thread(void*);
void* alarm_thread(void*);

 
int main(int argc, char *argv[]){

    data_t data = {
        .quit = false,
        .in_pipe = -1,
        .out_pipe = -1,
        .send_char = ' ',
        .received_char = '?',
        .led = false,
        .alarm_period = 0,
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
    data_t *data = (data_t*) arg;

    pthread_mutex_lock(&mtx);
    bool q = data->quit;
    pthread_mutex_unlock(&mtx);

    while (!q && (c = getchar()) != EOF) {
        pthread_mutex_lock(&mtx);
        switch(c) {
            case '1': case '2': case '3': case '4': case '5':
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
        q = data->quit;
        pthread_mutex_unlock(&mtx);
        pthread_cond_signal(&cond);
    }
    return &ret;
}
void* input_thread_pipe(void *arg){
    data_t *data = (data_t*) arg;
    static int ret = 0;

    pthread_mutex_lock(&mtx);
    bool q = data->quit;
    pthread_mutex_unlock(&mtx);
    unsigned char c;

    while (!q){
        int r = io_getc_timeout(data->in_pipe, READ_TIMEOUT_MS, &c);

        pthread_mutex_lock(&mtx);
        if (r > 0) {
            switch (c) {
                case 'b':
                    data->alarm_period = 0;
                    data->alarm_counter = 0;
                    data->led = false
                    data->quit = true;
                    break;
                case 'x':
                    data->led = true;
                    data->alarm_counter++;
                    break;
                case 'o':
                    data->led = false;
                    data->alarm_counter++;
                    break;
                case 'a':
                    if (data->send_char == 's'){
                        data->led = true;
                        data->alarm_period = 0;
                        data->alarm_counter = 0;
                    } else if (data->send_char == 'e') {
                        data->led = false;
                        data->alarm_period = 0;
                        data->alarm_counter = 0;
                    }
            }
            data->received_char = c;
            pthread_cond_signal(&cond);
        }
        q = data->quit;
        pthread_mutex_unlock(&mtx);

    }

    return &ret;
}

void* output_thread(void *arg){
    data_t *data = (data_t*) arg;
    static int ret = 0;

    pthread_mutex_lock(&mtx);
    bool q = data->quit;
    pthread_mutex_unlock(&mtx);

    while (!q){
        pthread_mutex_lock(&mtx);
        printf("\rLED %3s send: '%c' received: '%c', T = %4d ms, ticker = %4d", data->led ? "On" : "Off", data->send_char, data->received_char, data->alarm_period, data->alarm_counter / 2);
        fflush(stdout);
        q = data->quit;
        pthread_cond_wait(&cond, &mtx);
        pthread_mutex_unlock(&mtx);
    }

    return &ret;

}
void* alarm_thread(void *arg){
    data_t *data = (data_t*) arg;
    static int ret = 0;

    pthread_mutex_lock(&mtx);
    bool q = data->quit;
    pthread_mutex_unlock(&mtx);

    while (!q){
        sleep(PERIOD_SEC);
        pthread_mutex_lock(&mtx);
        if (data->alarm_counter > 0){
            data->alarm_period = (PERIOD_SEC * 1000 / data->alarm_counter);
        } else {
            data->alarm_period = 0;
        }
        data->alarm_counter = 0;
        q = data->quit;
        pthread_mutex_unlock(&mtx);

    }

    return &ret;
}




/* end of prga-hw08-main.c */
