#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
int N;
int partition = 0;
double sum = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* calcPI(void* threadid) {
    int id = *(int*)threadid;
    int L = partition * id;
    for(int i = L; i < L + partition; ++i){
		double x = (double)i/N;
        if(i > N) break;
        pthread_mutex_lock(&mutex);
		sum += sqrt( 1 - x*x ) / N ;
        pthread_mutex_unlock(&mutex);
	}
    pthread_exit(NULL);
}


int main(int argc, char *argv[]){

    assert(argc == 3);
    int num_threads = atoi(argv[1]);
    pthread_t threads[num_threads];

    N = atoi(argv[2]);
    partition = ceil(1.0 * N / num_threads);
    
    int rc, t;
    int ID[num_threads];
    for (t = 0; t < num_threads; ++t) {
        ID[t] = t;
        rc = pthread_create(&threads[t], NULL, calcPI, (void*)&ID[t]);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    for (t = 0; t < num_threads; ++t) pthread_join(threads[t], NULL);
    printf("%.10lf\n", sum * 4);

    pthread_exit(NULL);
	
}