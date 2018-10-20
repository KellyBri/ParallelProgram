#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

char isPrime(int N){
    int sqrt_N = sqrt(N);
    char is = 1;
    for(int i=2; i<=sqrt_N; ++i){
        if(N%i==0){
            is = 0;
            break;
        }
    }
    return is;
}

int main(int argc, char *argv[]){

    int N = atoi(argv[1]);
    int count = 0;
    // Beginning of parallel section. Fork a team of threads.
    #pragma omp parallel for schedule(dynamic, 1)
    
        //#pragma omp for schedule(dynamic, 5) nowait
        for (int i=2; i<N; ++i){
            if(isPrime(i))
                #pragma omp critical 
                ++count;
        }
        
    printf("\n%d\n", count);
    return 0;
}