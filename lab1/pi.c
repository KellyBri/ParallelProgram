#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

	int rc, rank, numtasks;
	rc = MPI_Init(&argc, &argv);

	if(rc != MPI_SUCCESS){
		printf("Error starting MPI program. Terminating.\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	

	double recv_buf = 0;
	double local_buf = 0;

	int N = atoi(argv[1]);
	int partition = ceil(1.0 * N / numtasks);
	int L = partition * rank;
	
	for(int i = L; i < L + partition; ++i){
		double x = (double)i/N;
		local_buf += sqrt( 1 - x*x ) / N ;
	}
	
	MPI_Reduce(&local_buf, &recv_buf, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	if(rank == 0) printf("%.10lf\n", recv_buf * 4);
	MPI_Finalize();
	return 0;
}
