#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <random>
#include <ctime>

#include "internal_config.hpp"
#include "GraphGen_notSorted.hpp"
#include "Square.hpp"
#include "Edge.hpp"
#include "threadsafe_queue.hpp"
#include "capacity_controller.hpp"
#include "utils.hpp"


void fill_up_edge_vector(
		Square& rec,
		std::vector<Edge>& edgeVector,
		std::uniform_int_distribution<>& dis, std::mt19937_64& gen,
		const double RMAT_a, const double RMAT_b, const double RMAT_c, const bool allowEdgeToSelf, const bool allowDuplicateEdges, const bool directedGraph
		) {

	std::vector<unsigned long long> throwAwayEdgesIndices; // Indices for those edges that must be eliminated due to duplicates and/or having same destination and source index.

	edgeVector.reserve(rec.getnEdges());
	generate_edges( std::ref(rec), std::ref(edgeVector), RMAT_a, RMAT_b, RMAT_c, directedGraph, allowEdgeToSelf, std::ref(dis), std::ref(gen), std::ref(throwAwayEdgesIndices) );

	if( !allowDuplicateEdges ) {

		do{
			// Resize the throw away edges vector for a fresh begin.
			throwAwayEdgesIndices.resize(0);

			std::sort( edgeVector.begin(), edgeVector.end() );

			// Detection of Invalid edges.
			for( unsigned long long edgeIdx = 0; edgeIdx < (edgeVector.size()-1); ++edgeIdx )
				if( edgeVector.at(edgeIdx) == edgeVector.at(edgeIdx+1) )
					throwAwayEdgesIndices.push_back(edgeIdx);

			// Add instead of eliminated and check until generate enough.
			if( !throwAwayEdgesIndices.empty() )
				generate_edges( std::ref(rec), std::ref(edgeVector), RMAT_a, RMAT_b, RMAT_c, directedGraph, allowEdgeToSelf, std::ref(dis), std::ref(gen), std::ref(throwAwayEdgesIndices) );

		} while( !throwAwayEdgesIndices.empty() );

	}

}

bool GraphGen_notSorted::GenerateGraph(
		const unsigned long long nEdges,
		const unsigned long long nVertices,
		const double RMAT_a, const double RMAT_b, const double RMAT_c,
		const unsigned int nCPUWorkerThreads,
		std::ofstream& outFile,
		const unsigned long long standardCapacity,
		const bool allowEdgeToSelf,
		const bool allowDuplicateEdges,
		const bool directedGraph
		) {

	std::vector<Square> squares ( 1, Square( 0, nVertices, 0, nVertices, nEdges, 0, 0, 0 ) );

	bool allRecsAreInRange;
	do {
		allRecsAreInRange = true;
		unsigned int recIdx = 0;
		for( auto& rec: squares ) {
			if( Eligible_RNG_Rec(rec, standardCapacity) ) {
				continue;
			} else {
				ShatterSquare(squares, RMAT_a, RMAT_b, RMAT_c, recIdx, directedGraph);
				allRecsAreInRange = false;
				break;
			}
			++recIdx;
		}
	} while( !allRecsAreInRange );

	// Making sure there are enough squares to utilize all threads.
	while( squares.size() < nCPUWorkerThreads && !edgeOverflow(squares) ) {
		// Shattering the biggest rectangle.
		unsigned long long biggest_size = 0;
		unsigned int biggest_index = 0;
		for( unsigned int x = 0; x < squares.size(); ++x )
			if( squares.at(x).getnEdges() > biggest_size ) {
				biggest_size = squares.at(x).getnEdges();
				biggest_index = x;
			}
		ShatterSquare(squares, RMAT_a, RMAT_b, RMAT_c, biggest_index, directedGraph);
	}

	if( SHOW_SQUARES_DETAILS )
		for( unsigned int x = 0; x < squares.size(); ++x )
			std::cout << squares.at(x);

	std::cout << squares.size() << " partition(s) specified." << "\n";
	std::cout << "Generating the graph ..." << "\n";

	std::vector<std::thread> threads;

	if( USE_A_MUTEX_TO_CONTROL_WRITE_TO_FILE_INSTEAD_OF_CONCURRENT_QUEUES ) {

		/*******************************************
		 * Control writes to file using a mutex
		 *******************************************/

		std::mutex writeMutex;
		unsigned int nWorkerThreads = nCPUWorkerThreads <squares.size() ? nCPUWorkerThreads : squares.size();

		auto eachThreadGenEdgesUsingMutexFunc = [&] ( unsigned int puId ) {
			std::vector<Edge> edgeVector;

			std::random_device rd;
			std::mt19937_64 gen(rd());
			std::uniform_int_distribution<> dis;

			for( unsigned int recIdx = puId; recIdx < squares.size(); recIdx += nWorkerThreads ) {
				auto& rec = squares.at( recIdx );
				fill_up_edge_vector( std::ref(rec), std::ref(edgeVector), std::ref(dis), std::ref(gen), RMAT_a, RMAT_b, RMAT_c, allowEdgeToSelf, allowDuplicateEdges, directedGraph );
				{
					std::lock_guard<std::mutex> guard( writeMutex );
					printEdgeGroup( std::ref(edgeVector), outFile );
					progressBar();
				}	// guard gets out-of-scope, unlocking the mutex upon destruction.
				edgeVector.clear();	// clean the edge vector for the next round
			}
		};

		for( unsigned int puIdx = 0; puIdx < nWorkerThreads; ++puIdx )
			threads.push_back( std::thread( eachThreadGenEdgesUsingMutexFunc, puIdx ) );

		std::for_each( threads.begin(), threads.end(), std::mem_fn(&std::thread::join) );

	} else {

		/*****************************************************
		 * Control writes to file using concurrent queues
		 *****************************************************/

		threadsafe_queue<Square> rec_queue;
		threadsafe_queue< std::vector<Edge> > EV_queue;
		capacity_controller<long long> capacityGate(static_cast<long long>(standardCapacity), 0);

		for( auto& rec: squares )
			rec_queue.push(rec);

		auto eachThreadGenEdgesUsingQueuesFunc = [&] {
			std::random_device rd;
			std::mt19937_64 gen(rd());
			std::uniform_int_distribution<> dis;
			Square rec;
			while( rec_queue.try_pop(std::ref(rec)) != 0 ) {
				capacityGate.accumulate( rec.getnEdges() );
				std::vector<Edge> edgeVector;
				fill_up_edge_vector( std::ref(rec), std::ref(edgeVector), std::ref(dis), std::ref(gen), RMAT_a, RMAT_b, RMAT_c, allowEdgeToSelf, allowDuplicateEdges, directedGraph );
				EV_queue.push(std::move(edgeVector));
			}
		};

		for( unsigned int puIdx = 0; puIdx < nCPUWorkerThreads; ++puIdx )
			threads.push_back( std::thread( eachThreadGenEdgesUsingQueuesFunc ) );

		std::vector<Edge> poppedEV;
		for( unsigned nWrittenEV = 0; nWrittenEV < squares.size(); ++nWrittenEV ) {
			EV_queue.wait_and_pop( std::ref(poppedEV) );
			printEdgeGroupNoFlush( poppedEV, outFile );
			capacityGate.dissipate( poppedEV.size() );
			progressBar();
		}

		std::for_each( threads.begin(), threads.end(), std::mem_fn(&std::thread::join) );

	}

	progressBarNewLine();

	return( EXIT_SUCCESS );

}
