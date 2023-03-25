//
// Starting code for the MPI coursework.
//
// First make sure mpicc is available ('module load mpi/mpich-x86_64' on school machines),
// then compile with:
//
// mpicc -Wall -o cwk2 cwk2.c
//
// or use the provided makefile. "-Wall"  turns on all warnings (optional but recommended).
// You can also add "-stc=c99" if you like.
//
 

//
// Includes
//
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

// Some extra routines for this coursework. DO NOT MODIFY OR REPLACE THESE ROUTINES,
// as this file will be replaced with a different version for assessment.
#include "cwk2_extra.h"

//
// Main
//
int main( int argc, char **argv )
{
	int i;

	// Initialise MPI and get the rank and no. of processes.
	int rank, numProcs;
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &numProcs );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank     );

	int *image = NULL, maxValue, dataSize;
	int pixelsPerProc;

	// Read in the image file to rank 0.
	if (rank == 0){
		// Read in the file and extract the maximum grey scale value and the data size (including padding bytes).
		// Defined in cwk2_extras.h; do not change, although feel free to inspect.
		image = readImage( "image.pgm", &maxValue, &dataSize, numProcs );
		if( image==NULL )
		{
			MPI_Finalize();
			return EXIT_FAILURE;
		}
		
		// The image size has already been rounded up to a multiple of numProcs by "readImage()".
		pixelsPerProc = dataSize / numProcs;

		printf("Rank 0: Read in PGM image array of size %d (%d per process), with max value %d.\n", dataSize, pixelsPerProc, maxValue );
	}

	
	// Allocate memory for the final histogram on rank 0.
	int *combinedHist = NULL;
	int *localHist = NULL;
	int *localImage = NULL;
	if (rank==0){		
		combinedHist = (int*) malloc( (maxValue+1)*sizeof(int) );				// Note 'maxValue+1', as range is 0 to maxValue inclusive.
		if( !combinedHist ) return allocateFail( "global histogram", rank );
		for(i=0; i<maxValue+1; i++ ) combinedHist[i] = 0;                        // Clear the histogram to zeroes.
	}
	MPI_Bcast(&pixelsPerProc, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&maxValue, 1, MPI_INT, 0, MPI_COMM_WORLD);

	localImage = (int*) malloc((pixelsPerProc+1)*sizeof(int));
	if( !localImage ) return allocateFail( "local image", rank );
	// for (int i=0; i<pixelsPerProc+1; i++) localImage[i] = 0;

	localHist = (int*) malloc((maxValue+1)*sizeof(int));
	if( !localHist ) return allocateFail( "local histogram", rank );
	for (int i=0; i<maxValue+1; i++) localHist[i] = 0;
	 
	// MPI_Recv(local_image, pixelsPerProc, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	// Start the timing.
	double startTime = MPI_Wtime();

	//
	// Your parallel code goes here. Feel free to change anything else in this file,
	// EXCEPT calls to the routines in cwk2_extra.h which MUST be called as provided.
	//

	if (numProcs && ((numProcs&(numProcs-1))==0)){
		int lev = 1;
		int p = 1;
		while (1<<lev<=numProcs)lev++;
		// printf("lev = %i\n",lev);
		
		for (int i = 0; i<(lev-1); i++){
			for (int j = 0; j<(1<<i); j++){
				// printf("Level: %i - sending from rank: %i to rank: %i (2^i=%i)\n", i, j, p,(1<<i));

				if (rank == j) {
					printf("Level: %i - sending from rank: %i to rank: %i (2^i=%i)\n", i, j, p,(1<<i));
					// printf("Sending");
					// MPI_Send(&image[i*pixelsPerProc], pixelsPerProc, MPI_INT, p, 0, MPI_COMM_WORLD);
				}
				p++;

				if (rank == p){ 
					// printf("Rank: %i is receiving from rank: %i\n",p,j);
					// MPI_Recv(localImage, pixelsPerProc, MPI_INT, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				}
			}
		}
	}
	else{
		// MPI_Bcast(&pixelsPerProc, 1, MPI_INT, 0, MPI_COMM_WORLD);
		// MPI_Bcast(&maxValue, 1, MPI_INT, 0, MPI_COMM_WORLD);

		// localImage = (int*) malloc((pixelsPerProc+1)*sizeof(int));
		// if( !localImage ) return allocateFail( "local image", rank );

		// localHist = (int*) malloc((maxValue+1)*sizeof(int));
		// if( !localHist ) return allocateFail( "local histogram", rank );
		// for (int i=0; i<maxValue+1; i++) localHist[i] = 0;	
		
		MPI_Scatter(image, pixelsPerProc, MPI_INT, localImage, pixelsPerProc, MPI_INT, 0, MPI_COMM_WORLD);
	}

	for(int i=0; i<pixelsPerProc; i++ ){
		if(localImage[i]>=0) localHist[localImage[i]]++;
	}
	MPI_Reduce(localHist, combinedHist, maxValue+1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	free(localHist);
	free(localImage);
	
	//
	// Outputs the time taken, and checks the histogram against the serial calculation. Only rank 0 involved.
	//
	if( rank==0 )
	{
		// Time for the parallel computation.
		printf( "Parallel calculation took a total time %g s.\n", MPI_Wtime() - startTime );

		// Allocate memory for the check histogram, and then initialise it to zero.
		int *checkHist = (int*) malloc( (maxValue+1)*sizeof(int) );
		if( !checkHist ) return allocateFail( "histogram for checking", rank );
		for( i=0; i<maxValue+1; i++ ) checkHist[i] = 0;

		// Construct the histogram using the global data only.
		for( i=0; i<dataSize; i++ )
			if( image[i]>=0 ) checkHist[ image[i] ]++;
		
		// Display the histgram.
		// for( i=0; i<maxValue+1; i++ ){
		// 	printf( "Greyscale value %i:\tCount %i\t(check: %i)\n", i, combinedHist[i], checkHist[i] );
		// 	if (combinedHist[i] != checkHist[i])
		// 		printf("UNSUCCESSFUL: %i != %i\n",combinedHist[i], checkHist[i]);
		// }
		free( checkHist );
	}
	
	//
	// Clear up and quit.
	//
	if( rank==0 )
	{
		saveHist( combinedHist, maxValue );		// Defined in cwk2_extra.h; do not change or replace this call.
		free( image );
		free( combinedHist );
	}
	
	MPI_Finalize();
	return EXIT_SUCCESS;
}
