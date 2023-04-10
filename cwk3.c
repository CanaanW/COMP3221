//
// Starting point for the OpenCL coursework for COMP3221 Parallel Computation.
//
// Once compiled, execute with the size of the vector, N, which is also the number of both
// rows and columns of the (square) matrix. N should be a power of 2, otherwise an error
// will be returned. Example usage is
//
// ./cwk3 8
//
// This will display the randomly-generated 8x8 matrix, a randonly-generated 8-vector, and the
// output 8-vector which currently is incorrect. You need to implement OpenCL code to perform
// the operation correctly.
//


//
// Includes.
//
#include <stdio.h>
#include <stdlib.h>

// For this coursework, the helper file has 3 routines in addition to simpleOpenContext_GPU() and compileKernelFromFile():
// - getCmdLineArgs()         : Gets the command line argument and checks they are valid.
// - displayMatrixAndVector() : Displays the matrix and the vector, or just the top-left corner if it is too large.
// - fillMatrixAndVector()    : Fills the matrix with random values.
// - displaySolution()        : Displays the solution vector, i.e. the matrix multiplied by the initial vector.
// Do not alter these routines, as they will be replaced with different versions for assessment.
#include "helper_cwk.h"


//
// Main.
//
int main( int argc, char **argv )
{
 
    //
    // Parse command line arguments and check they are valid. Handled by a routine in the helper file.
    //
    int N;
    getCmdLineArgs( argc, argv, &N );

    //
    // Initialisation.
    //

    // Set up OpenCL using the routines provided in helper_cwk.h.
    cl_device_id device;
    cl_context context = simpleOpenContext_GPU(&device);

    // Open up a single command queue, with the profiling option off (third argument = 0).
    cl_int status;
    cl_command_queue queue = clCreateCommandQueue( context, device, 0, &status );

    // Allocate memory for the vector and the matrix.
    float *hostMatrix   = (float*) malloc( N*N * sizeof(float) );
	float *hostVector   = (float*) malloc( N   * sizeof(float) );
	float *hostSolution = (float*) malloc( N   * sizeof(float) );

    // Fill the matrix and vector with random values, and display.
	fillMatrixAndVector( hostMatrix, hostVector, N );
    displayMatrixAndVector( hostMatrix, hostVector, N );		// DO NOT ALTER; Your solution MUST call this function at the start of your calculation.

	// Initialise the solution vector to zero.
	int i;
	for( i=0; i<N; i++ ) hostSolution[i] = 0.0f;

    //
    // Perform the calculation on the GPU.
    //

    int row, col;
        for( row=0; row<N; row++ )
            for( col=0; col<N; col++ )
                y[row] += M[row*N+col] * x[col];
	// Your solution should mainly go here and in the associated .cl file,
	// but you are free to make changes elsewhere in this code.


    //
    // Display the final result.
    //
    displaySolution( hostSolution, N );			// DO NOT ALTER; Your solution MUST call this function at the end of your calculation.

    //
    // Release all resources.
    //
    clReleaseCommandQueue( queue   );
    clReleaseContext     ( context );

    free( hostMatrix   );
	free( hostVector   );
	free( hostSolution );
 
    return EXIT_SUCCESS;
}

