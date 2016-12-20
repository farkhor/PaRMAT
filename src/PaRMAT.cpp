#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <stdexcept>
#include <thread>
#include <algorithm>
#include <cstdlib>

#include "GraphGen_sorted.hpp"
#include "GraphGen_notSorted.hpp"
#include "utils.hpp"
#include "internal_config.hpp"


void openFileToWrite( std::ofstream& input_file, const char* file_name ) {
	input_file.open(file_name);
	if( !input_file )
		throw std::runtime_error( "Failed to open specified file." );
}

template <typename intT>
intT determine_num_of_CPU_worker_thread( const intT suggested ) {
	if( suggested < MIN_CPU_WORKER_THREAD )
		return static_cast<intT>( MIN_CPU_WORKER_THREAD );
	else if( suggested > MAX_CPU_WORKER_THREAD )
		return static_cast<intT>( MAX_CPU_WORKER_THREAD );
	else
		return static_cast<intT>( suggested );
}

double determine_RAM_usage( const double suggested ) {
	if( suggested < MIN_RAM_PORTION_USAGE )
		return MIN_RAM_PORTION_USAGE;
	else if( suggested > MAX_RAM_PORTION_USAGE )
		return MAX_RAM_PORTION_USAGE;
	else
		return suggested;
}

int main( int argc, char ** argv ) {

	std::string usage =
	"\tRequired command line arguments:\n\
		-Number of edges. E.g., -nEdges 1021\n\
		-NUmber of vertices. E.g., -nVertices 51\n\
	Additional arguments:\n\
		-Output file (default: out.txt). E.g., -output myout.txt\n\
		-RMAT a parameter (default: 0.45). E.g., -a 0.42\n\
		-RMAT b parameter (default: 0.22). E.g., -b 0.42\n\
		-RMAT c parameter (default: 0.22). E.g., -c 0.42\n\
		-Number of worker CPU threads (default: queried/1). E.g., -threads 4\n\
		-Output should be sorted based on source index (default: not sorted). To sort: -sorted\n\
		-Allow edge to self (default:yes). To disable: -noEdgeToSelf\n\
		-Allow duplicate edges (default:yes). To disable: -noDuplicateEdges\n\
		-Will the graph be directed (default:yes). To make it undirected: -undirected\n\
		-Usage of available system memory (default: 0.5 which means up to half of available RAM may be requested). E.g., -memUsage 0.9";


	std::ofstream outf;
	unsigned long long nEdges = 0, nVertices = 0;
	double a = 0.45, b = 0.22, c = 0.22;
	unsigned int nCPUWorkerThreads = 0;
	bool sorted = false;
	double RAM_usage = 0.5;
	bool allowEdgeToSelf = true;
	bool allowDuplicateEdges = true;
	bool directedGraph = true;
	unsigned long long standardCapacity = 0;

	try{

		// Getting required input parameters.
		for( int iii = 1; iii < argc; ++iii ) {
			if( !strcmp(argv[iii], "-nEdges") && iii != argc-1 /*is not the last one*/)
				nEdges = std::stoull( std::string(argv[iii+1]) );
			else if( !strcmp(argv[iii], "-nVertices") && iii != argc-1 /*is not the last one*/)
				nVertices = std::stoull( std::string(argv[iii+1]) );
			else if( !strcmp(argv[iii], "-output") && iii != argc-1 /*is not the last one*/)
				openFileToWrite( outf, argv[iii+1] );
			else if( !strcmp(argv[iii], "-a") && iii != argc-1 /*is not the last one*/)
				a = std::stod( std::string(argv[iii+1]) );
			else if( !strcmp(argv[iii], "-b") && iii != argc-1 /*is not the last one*/)
				b = std::stod( std::string(argv[iii+1]) );
			else if( !strcmp(argv[iii], "-c") && iii != argc-1 /*is not the last one*/)
				c = std::stod( std::string(argv[iii+1]) );
			else if( !strcmp(argv[iii], "-threads") && iii != argc-1 /*is not the last one*/)
				nCPUWorkerThreads = determine_num_of_CPU_worker_thread( std::stoul( std::string(argv[iii+1]) ) );
			else if( !strcmp(argv[iii], "-sorted"))
				sorted = true;
			else if( !strcmp(argv[iii], "-memUsage") && iii != argc-1 /*is not the last one*/)
				RAM_usage = determine_RAM_usage( std::stod( std::string(argv[iii+1]) ) );
			else if( !strcmp(argv[iii], "-noEdgeToSelf"))
				allowEdgeToSelf = false;
			else if( !strcmp(argv[iii], "-noDuplicateEdges"))
				allowDuplicateEdges = false;
			else if( !strcmp(argv[iii], "-undirected"))
				directedGraph = false;
		}

		if( nVertices == 0 || nEdges == 0 || nEdges >= nVertices*nVertices )
			throw std::runtime_error( "Number of edges or number of vertices are not specified (correctly)." );

		if( !outf.is_open() )
			openFileToWrite( outf, "out.txt" );

		// Avoiding very small regions which may cause incorrect results.
		if( nEdges < 10000 )
			nCPUWorkerThreads = 1;

		if( nCPUWorkerThreads == 0 )	// If number of concurrent threads haven't specified by the user,
			nCPUWorkerThreads = std::max( 1, static_cast<int>(std::thread::hardware_concurrency()) - 1 );	// try to manage their numbers automatically. If cannot determine, go single-threaded.

		// Print the info.
		std::cout << "Requested graph will have " << nEdges << " edges and " << nVertices << " vertices." << "\n" <<
				"Its a, b, and c parameters will be respectively " << a << ", " << b << ", and " << c << "." << "\n" <<
				"There can be up to " << nCPUWorkerThreads << " worker CPU thread(s) making the graph." << "\n" <<
				"The graph will" << (sorted?" ":" NOT ") << "necessarily be sorted." << "\n" <<
				"Up to about "<< (RAM_usage*100.0) << " percent of RAM can be used by this program." << "\n" <<
				"Specified graph may" << (allowEdgeToSelf?" ":" NOT ") << "contain edges that have same source and destination index." << "\n" <<
				"Specified graph may" << (allowDuplicateEdges?" ":" NOT ") << "contain duplicate edges." << "\n" <<
				"Specified graph will be " << (directedGraph?"DIRECTED.":"UNDIRECTED.") << "\n";

		auto totalSystemRAM = static_cast<unsigned long long>(getTotalSystemMemory());	// In bytes.
		auto availableSystemRAM = calculateAvailableRAM( totalSystemRAM, RAM_usage );	// In bytes.

		standardCapacity = availableSystemRAM / (2*nCPUWorkerThreads*sizeof(Edge)); // 2 can count for vector's effect.
		std::cout << "Each thread capacity is " << standardCapacity << " edges." << "\n";

	}
	catch( const std::exception& strException ) {
		std::cerr << "Initialization Error: " << strException.what() << "\n";
		std::cerr << "Usage: " << usage << std::endl << "Exiting." << std::endl;
		return( EXIT_FAILURE );
	}
	catch(...) {
		std::cerr << "An exception has occurred during the initialization." << "\n" << "Exiting." << std::endl;
		return( EXIT_FAILURE );
	}

	try{

		// Start the work.
		--nVertices;
		auto fOutcome = sorted ?	GraphGen_sorted::GenerateGraph( nEdges, nVertices, a, b, c, nCPUWorkerThreads, outf, standardCapacity, allowEdgeToSelf, allowDuplicateEdges, directedGraph ) :
									GraphGen_notSorted::GenerateGraph( nEdges, nVertices, a, b, c, nCPUWorkerThreads, outf, standardCapacity, allowEdgeToSelf, allowDuplicateEdges, directedGraph );
		if( fOutcome == EXIT_FAILURE ) {
			std::cerr << "Exiting." << std::endl;
			return( EXIT_FAILURE );
		}

		std::cout << "Done." << std::endl;
		return( EXIT_SUCCESS );

	}
	catch( const std::exception& strException ) {
		std::cerr << "Error: " << strException.what() << "\n" << "Exiting." << std::endl;
		return( EXIT_FAILURE );
	}
	catch(...) {
		std::cerr << "An exception has occurred during the graph generation." << "\n" << "Exiting." << std::endl;
		return( EXIT_FAILURE );
	}

}
