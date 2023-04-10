//
// This is a modified version of helper.h that has additional routines specific to the coursework.
// As with previous courseworks, this will be replaced with a different version of helper_cwk.h for
// assessment; therefore:
// - Do NOT change this file (any such changes will be lost during assessment).
// - Do NOT copy any of this file into you code.
// - Do NOT reproduce any of this file in your own code.
//


// Need OpenCL; how to include depends on the OS.
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

// Other libraries needed here.
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



//
// Gets the sole command line argument, and performs some basic error checking.
//
void getCmdLineArgs( int argc, char **argv, int *N )
{
    if( argc != 2 )
    {
        printf( "Require exactly 1 command line argument: The size of the vector N and the NxN matrix. Must be a power of 2.\n" );
        exit( EXIT_FAILURE );
    }

	*N = atoi(argv[1]);

    if( *N<=0 )
    {
        printf( "N must be a positive integer.\n" );
        exit( EXIT_FAILURE );
    }

    // Require N to be a power of 2 (some older devices may require this).
    if( (*N&(*N-1))!=0 )
    {
        printf( "N must be a power of 2.\n" );
        exit( EXIT_FAILURE );
    }
}


//
// Fills the matrix and vector with random values.
//
void fillMatrixAndVector( float *M, float *x, int N )
{
	int row, col;

	srand( time(0) );

	for( row=0; row<N; row++ )
	{
		for( col=0; col<N; col++ )
			M[row*N+col] = 1.0 * rand() / RAND_MAX;
		
		x[row] = 1.0 * rand() / RAND_MAX;
	}
}

//
// Displays the matrix M and the vector x. Only show a part if N is large.
//
void displayMatrixAndVector( float *M, float *x, int N )
{
    int i, j;

	// Display the matrix.
	printf( "The %ix%i matrix", N, N );
	if( N>8 ) printf( " (only showing top-left 8x8 elements)" );
	printf( ":\n" );

    // Only display the top-left corner if large, with ellipses '...' to suggest continuation.
    for( i=0; i<(N<8?N:8); i++ )
    {
        for( j=0; j<(N<8?N:8); j++ ) printf( "%.3f ", M[i*N+j] );
        if( N>8 ) printf( ". . ." );
        printf( "\n" );
    }

    // Ellipses at the bottom of the matrix.
    if( N>8 )
        for( i=0; i<3; i ++ )
        {
            for( j=0; j<(N<8?N:8); j++ ) printf( "  .   " );
            printf( "\n" );
        }

    printf( "\n" );

	// Display the vector.
	printf( "The %i-vector", N );
	if( N>8 ) printf( " (only showing first 8 elements)" );
	printf( ", displayed as a row vector for convenience:\n" );

	for( i=0; i<(N<8?N:8); i++ )
		printf( "%.3f ", x[i] );
	
	if( N>8 ) printf( ". . . " );

	printf( "\n" );
}

//
// Displays the solution vector. Only show a part if N is large.
//
void displaySolution( float *y, int N )
{
    int i;

	printf( "\nThe solution (also a %i-vector)", N );
	if( N>8 ) printf( " (only showing first 8 elements)" );
	printf( ":\n" );

	for( i=0; i<(N<8?N:8); i++ )
		printf( "%.3f ", y[i] );
	
	if( N>8 ) printf( ". . . " );

	printf( "\n" );
}



//
//	Tries to open up a single GPU on the first OpenCL framework, returning the context	
//	and filling the passed device i.d.
//	
//	Fails with a brief error message and calls exit(EXIT_FAILURE) if there was some problem.
// 
cl_context simpleOpenContext_GPU( cl_device_id *device )
{
	// Status; returned/modified after each API call; zero if successful.
	cl_int status;

	//
	// Get all of the platforms.
	//

	// This is straight from displayDevices.c
	cl_uint platformCount;
	status = clGetPlatformIDs( 0, NULL, &platformCount );                           // Entries; platform array; no. platforms
	if( status )
	{
			printf( "clGetPlatformIDs returned error %i (when trying to access no. of platforms).\n", status );
			exit(-1);
	}

	// Need at least one platform.
	if( platformCount==0 )
	{
		printf( "Could not find any OpenCL platforms.\n" );
		exit(-1);
	}

	// Get all of the platform IDs.
	cl_platform_id *platformIDs = (cl_platform_id*) malloc( platformCount*sizeof(cl_platform_id) );
	status = clGetPlatformIDs( platformCount, platformIDs, NULL );          // Call again; different arguments
	if( status )
	{
			printf( "clGetPlatformIDs returned error %i (when trying extract platform IDs)\n", status );
			exit(-1);
	}

	//
	// Loop through all platforms.
	//
	int platNum;
	for( platNum=0; platNum<platformCount; platNum++ )
	{
		// Get the first GPU for this platform. Skip to the next platform if it does not appear to have a GPU.
		cl_uint numGPUs;
		status = clGetDeviceIDs( platformIDs[platNum], CL_DEVICE_TYPE_GPU, 0, NULL, &numGPUs );
		if( numGPUs==0 || status==CL_DEVICE_NOT_FOUND ) continue;			// The statis check seems to help on Windows.

		// Get the device ID for all GPUs, and take the first.
		cl_device_id *GPUIDs = (cl_device_id*) malloc( numGPUs*sizeof(cl_device_id) );
		status = clGetDeviceIDs( platformIDs[platNum], CL_DEVICE_TYPE_GPU, numGPUs, GPUIDs, NULL );
		if( status != CL_SUCCESS )
		{
			printf( "Failed to get a viable GPU ID.\n" );
			exit( EXIT_FAILURE );
		}
		*device = GPUIDs[0];			// Use the first one.
		free( GPUIDs );

		// Create a context and associate it with this device.
		cl_context context = clCreateContext( NULL, 1, device, NULL, NULL, &status );

		return context;
	}

	// If still here, did not find a GPU anywhere.
	printf( "Could not find an OpenCL-compliant GPU on any platform.\n" );
	exit(-1);
}


//
//	Attempts to load and compile an OpenCL kernel with the given filename; also need a name,
//	the context and the device.
//
//  Prints a brief error message and calls exit( EXIT_FAILURE ) if there was some problem.
//
cl_kernel compileKernelFromFile( const char *filename, const char *kernelName, cl_context context, cl_device_id device )
{
	FILE *fp;
	char *fileData;
	long fileSize;
	
	// Open the file.
	fp = fopen( filename, "r" );
	if( !fp )
	{
		printf( "Could not open the file '%s'\n", filename );
		exit( EXIT_FAILURE );
	}
	
	// Get the file size; also check it is positive.
	if( fseek(fp,0,SEEK_END) )
	{
		printf( "Could not extract the file size from '%s'.\n", filename );
		exit( EXIT_FAILURE );
	}
	fileSize = ftell(fp);
	if( fileSize<1 )
	{
		printf( "Could not read file (or zero size) for file '%s'.\n", filename );
		exit( EXIT_FAILURE );
	}
	
	// Move to the start of the file.
	if( fseek(fp,0,SEEK_SET) )
	{
		printf( "Error reading the file '%s' (could not move to start).\n", filename );
		exit( EXIT_FAILURE );
	}
	
	// Read the contents; also need a termination character.
	fileData = (char*) malloc( fileSize+1 );
	if( !fileData )
	{
		printf( "Could not allocate memory for the character buffer.\n" );
		exit( EXIT_FAILURE );
	}
	if( fread(fileData,fileSize,1,fp) != 1 )
	{
		printf( "Error reading the file '%s'.\n", filename );
		exit( EXIT_FAILURE );
	}
	
	// Terminate the string.
	fileData[fileSize] = '\0';
	
	// Close the file.
	if( fclose(fp) )
	{
		printf( "Error closing the file '%s'.\n", filename );
		exit( EXIT_FAILURE );
	}
	
	// Now for the OpenCL-specific stuff. Create the program from the character string.
	cl_int status;
	cl_program program = clCreateProgramWithSource( context, 1, (const char**)&fileData, NULL, &status );
	if( status != CL_SUCCESS )
	{
		printf( "Failed to create program from the source '%s'.n", filename );
		exit( EXIT_FAILURE );
	}

	// Build the program.
	if( (status=clBuildProgram(program,1,&device,NULL,NULL,NULL)) != CL_SUCCESS )
	{
		printf( "Failed to build the kernel '%s' from the file '%s'; error code %i.\n", kernelName, filename, status );
		
		// Provide more information about the nature of the fail.
		size_t logSize;
		clGetProgramBuildInfo( program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize );
		char *log = (char*) malloc( (logSize+1)*sizeof(char) );
		
		clGetProgramBuildInfo( program, device, CL_PROGRAM_BUILD_LOG, logSize+1, log, NULL );
		printf( "Build log:\n%s\n", log );
		
		// Clear up and quit.
		free( log );
		exit( EXIT_FAILURE );
	}
	
	// Now compile it.
	cl_kernel kernel = clCreateKernel( program, kernelName, &status );
	if( status != CL_SUCCESS )
	{
		printf( "Failed to create the OpenCL kernel with error code %i.\n", status );
		
		// Common mistake(s).
		if( status==-46 ) printf( "Ensure the kernel name '%s' is also the name of the function.\n", kernelName );

		exit( EXIT_FAILURE );
	}
	
	// Clear up (not the kernel, which will have to be released by the caller).
	clReleaseProgram( program );
	free( fileData );
	
	return kernel;
}

