#ifndef INTERNAL_CONFIG_HPP
#define INTERNAL_CONFIG_HPP

/***************************************************************************
 * Compile-time program configuration can be partially controlled from here.
 ***************************************************************************/

// If you wish to have graphs that will have more than around 4 billion vertices, you should change below definition to stand for a bigger type; unsigned long long for instance.
#define EdgeIndexType unsigned int

// Uncomment below line if you would like to add some small noise to distribution of R-MAT graph parameters: a, b, and c.
//#define ADD_NOISE_TO_RMAT_PARAMETERS_AT_EACH_LEVEL

#define MIN_CPU_WORKER_THREAD 1
#define MAX_CPU_WORKER_THREAD 128

#define MIN_RAM_PORTION_USAGE 0.01
#define MAX_RAM_PORTION_USAGE 0.9

#define SHOW_PROGRESS_BARS
#define SHOW_SQUARES_DETAILS

//#define USE_FUTURES_INSTEAD_OF_EXPLICIT_THREADS								// Enabling it makes the sorted output get created by "futures" instead of using explicit threads.
//#define USE_A_MUTEX_TO_CONTROL_WRITE_TO_FILE_INSTEAD_OF_CONCURRENT_QUEUES		// Enabling it makes the non-sorted output get created by a mutex, that controls the exclusiveness of the writes to file, instead of using concurrent queues.

#endif	//	INTERNAL_CONFIG_HPP
