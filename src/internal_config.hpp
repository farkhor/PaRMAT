#ifndef INTERNAL_CONFIG_HPP
#define INTERNAL_CONFIG_HPP

/***************************************************************************
 * Compile-time program configuration can be partially controlled from here.
 ***************************************************************************/

// If you wish to have graphs that will have more than around 4 billion vertices, you should change below definition to stand for a bigger type; unsigned long long for instance.
using EdgeIndexType = unsigned int;

// Set below property if you would like to add some small noise to distribution of R-MAT graph parameters: a, b, and c.
static const bool ADD_NOISE_TO_RMAT_PARAMETERS_AT_EACH_LEVEL = false;

// Maximum and minimum number of worker threads.
static const unsigned int MIN_CPU_WORKER_THREAD = 1, MAX_CPU_WORKER_THREAD = 128;

// Maximum and minimum allowed usage of RAM.
static const double MIN_RAM_PORTION_USAGE = 0.01, MAX_RAM_PORTION_USAGE = 0.9;

// Set if you would like the progress bar to be shown during the process.
static const bool SHOW_PROGRESS_BARS = true;

// Set if you would like to see the details of assignment of edges to squares.
static const bool SHOW_SQUARES_DETAILS = false;

// Enabling it makes the sorted output get created by "futures" instead of using explicit threads.
static const bool USE_FUTURES_INSTEAD_OF_EXPLICIT_THREADS = false;

// Enabling it makes the non-sorted output get created by a mutex, that controls the exclusiveness of the writes to file, instead of using concurrent queues.
static const bool USE_A_MUTEX_TO_CONTROL_WRITE_TO_FILE_INSTEAD_OF_CONCURRENT_QUEUES = false;

#endif	//	INTERNAL_CONFIG_HPP
