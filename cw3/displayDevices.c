//
// Outputs information about all available devices
//
// Compile as per the instructions given in lectures.
//

// Includes
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

int main()
{
	// Error code
	cl_int status;

	//
	// Platforms
	//

	// Get all of the platform IDs
	cl_uint platformCount;
	status = clGetPlatformIDs( 0, NULL, &platformCount );				// Entries; platform array; no. platforms
	if( status )
	{
		printf( "clGetPlatformIDs returned error %i (when trying to access no. of platforms)\n", status );
		exit(-1);
	}
	printf( "Found %i platform(s).\n", platformCount );

	// Get all of the platforms
	cl_platform_id *platformIDs = (cl_platform_id*) malloc( platformCount*sizeof(cl_platform_id) );
	status = clGetPlatformIDs( platformCount, platformIDs, NULL );		// Call again; different arguments
	if( status )
	{
		printf( "clGetPlatformIDs returned error %i (when trying extract platform IDs)\n", status );
		exit(-1);
	}

	//
	// Loop over all platforms, getting the devices for each
	//
	int platNum;
	for( platNum=0; platNum<platformCount; platNum++ )
	{
		// Get the number of devices
		cl_uint deviceCount;
		status = clGetDeviceIDs( platformIDs[platNum], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount );
		if( status )
		{
			printf( "clGetDeviceIDs failed to extract the number of devices for this platform.\n" );
			exit( -1 );
		}
		printf( "\nPlatform %i has %i device(s).\n", platNum, deviceCount );

		// Get pointers to the devices; not these are destroyed at the end of the platform loop.
		cl_device_id *deviceIDs = (cl_device_id*) malloc( deviceCount*sizeof(cl_device_id) );
		status = clGetDeviceIDs( platformIDs[platNum], CL_DEVICE_TYPE_ALL, deviceCount, deviceIDs, NULL );
		if( status )
		{
			printf( "clGetDeviceIDs failed to extract the devices.\n" );
			exit( -1 );
		}

		//
		// Loop over devices; no more error checking for status
		//
		int devNum;
		for( devNum=0; devNum<deviceCount; devNum++ )
		{
			printf( "\nDevice %i:\n", devNum );

			// Get the device type first
			cl_device_type deviceType;
			status = clGetDeviceInfo( deviceIDs[devNum], CL_DEVICE_TYPE, sizeof(cl_device_type), &deviceType, NULL );
			switch( deviceType )
			{
				case CL_DEVICE_TYPE_CPU: printf( " - Device Type: CPU\n" ); break;
				case CL_DEVICE_TYPE_GPU: printf( " - Device Type: GPU\n" ); break;
				default:
					printf( " - type i.d. %d\n", (int)deviceType );
			}

			// Text entries can be done in a loop
			cl_device_info strInfoLabels[4] = { CL_DEVICE_NAME, CL_DEVICE_VERSION, CL_DRIVER_VERSION, CL_DEVICE_EXTENSIONS };
			int i;
			for( i=0; i<4; i++ )
			{
				// All of these return a string; first length, then the char*
				char* value;
				size_t valueSize;

				// Device name
				status = clGetDeviceInfo( deviceIDs[devNum], strInfoLabels[i], 0, NULL, &valueSize );
				value = (char*) malloc(valueSize);
				status = clGetDeviceInfo( deviceIDs[devNum], strInfoLabels[i], valueSize, value, NULL );

				switch( i )
				{
					case 0	:	printf( " - Device name: %s\n",    value ); break;
					case 1	:	printf( " - Device version: %s\n", value ); break;
					case 2	:	printf( " - Driver version: %s\n", value ); break;
					case 3	:	printf( " - Extensions: %s\n",     value ); break;

				}

				free(value);
			}

			// size_t types can similarly be performed in a loop
			cl_device_info uintInfoLabels[3] = { CL_DEVICE_MAX_COMPUTE_UNITS, CL_DEVICE_MAX_WORK_GROUP_SIZE, CL_DEVICE_ADDRESS_BITS };
			for( i=0; i<3; i++ )
			{
				size_t value;
				status = clGetDeviceInfo( deviceIDs[devNum], uintInfoLabels[i], sizeof(value), &value, NULL );

				switch( i )
				{
					case 0	:	printf( " - %i compute unit(s)\n",         (int)value ); break;
					case 1	:	printf( " - Max. group work size = %i\n",  (int)value ); break;
					case 2	:	printf( " - Address width is %i bits\n",   (int)value ); break;
				}
			}
 		}

		//
		// Clear up per-platform allocations
		//
		free( deviceIDs );
	}

	//
	// Clear up remaining allocations
	//
	free( platformIDs );
}