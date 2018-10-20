#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
int num[4];
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 


void* write() {
    printf("Enter 4 values\n");
    for (int i = 0; i < 4; ++i) scanf("%d", &num[i]);
    pthread_cond_signal(&cond);		
}

void* read() {    
    pthread_cond_wait( &cond ,&mutex);	
    printf("Values filled in array are\n");
    for (int i = 0; i < 4; ++i) printf("%d \n", num[i]);
    pthread_exit(NULL);
}

int main(void) {
    pthread_t threads[2];
    printf("Threads have been created\n");
    pthread_create(&threads[1], NULL, &read, NULL);
    pthread_create(&threads[0], NULL, &write, NULL);
    
    
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    return 0;
}
