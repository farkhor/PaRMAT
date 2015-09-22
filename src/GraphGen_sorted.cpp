#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>

#include "internal_config.hpp"
#include "GraphGen_sorted.hpp"
#include "Square.hpp"
#include "Edge.hpp"
#include "utils.hpp"


void EachThreadGeneratesEdges(
		std::vector<Square>& recs,
		std::vector<Edge>& edgesVec,
		const double RMAT_a, const double RMAT_b, const double RMAT_c,
		const bool allowEdgeToSelf, const bool allowDuplicateEdges, const bool directedGraph
		) {

	// First clean the edge vector before adding any.
	edgesVec.clear();

	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<> dis;

	// Reserve enough memory once.
	unsigned long long nEdgesToAllocate = 0;
	for( auto& rec: recs )
		nEdgesToAllocate += rec.getnEdges();
	edgesVec.reserve( nEdgesToAllocate );

	std::vector<unsigned long long> throwAwayEdgesIndices; // Indices for those edges that must be eliminated due to duplicates and/or having same destination and source index.

	// For all the rectangles we have to generate edge for.
	for( auto& rec: recs ) {

		generate_edges( std::ref(rec), std::ref(edgesVec), RMAT_a, RMAT_b, RMAT_c, directedGraph, allowEdgeToSelf, std::ref(dis), std::ref(gen), std::ref(throwAwayEdgesIndices) );

		if( !allowDuplicateEdges ) {

			// Sort the edge vector.
			std::sort( edgesVec.end()-rec.getnEdges(), edgesVec.end() );

			do{
				// Resize the throw away edges vector for a fresh begin.
				throwAwayEdgesIndices.resize(0);

				// Detection of Invalid edges.
				auto edgeIdx = edgesVec.end()-1;
				for(  ; edgeIdx > (edgesVec.end()-rec.getnEdges()); --edgeIdx )
					if( (*edgeIdx) == (*(edgeIdx-1)) )
						throwAwayEdgesIndices.push_back(edgeIdx - edgesVec.begin());

				if( !throwAwayEdgesIndices.empty() ) {
					// Add instead of eliminated and check until generate enough.
					generate_edges( std::ref(rec), std::ref(edgesVec), RMAT_a, RMAT_b, RMAT_c, directedGraph, allowEdgeToSelf, std::ref(dis), std::ref(gen), std::ref(throwAwayEdgesIndices) );
					// Sort edge vector if there are duplicates or self-edges.
					std::sort( edgesVec.end()-rec.getnEdges(), edgesVec.end() );
				}

			} while( !throwAwayEdgesIndices.empty() );

		}

	}

	std::sort( edgesVec.begin(), edgesVec.end() );

}

bool GraphGen_sorted::GenerateGraph(
		const unsigned long long nEdges,
		const unsigned long long nVertices,
		const double RMAT_a, double RMAT_b, double RMAT_c,
		const unsigned int nCPUWorkerThreads,
		std::ofstream& outFile,
		const unsigned long long standardCapacity,
		const bool allowEdgeToSelf,
		const bool allowDuplicateEdges,
		const bool directedGraph
		) {

	std::vector<Square> squares ( 1, Square( 0, nVertices, 0, nVertices, nEdges, 0, 0, 0 ) );

	while( !Eligible_RNG_Squares(squares, standardCapacity ) ) {
		// For sorted version, we break down all the rectangles each time there's a non-eligible rectangle.
		unsigned int nIters = squares.size();
		for( unsigned int x = 0; x < nIters; ++x )
			ShatterSquare(squares, RMAT_a, RMAT_b, RMAT_c, 0, directedGraph);
		std::sort( squares.begin(), squares.end() );
	}

	// Making sure there are enough columns of rectangles to utilize all threads.
	while( Get_N_Columns(squares) < nCPUWorkerThreads && !edgeOverflow(squares) ) {
		unsigned int nIters = squares.size();
		// Breaking all rectangles again.
		for( unsigned int x = 0; x < nIters; ++x )
			ShatterSquare(squares, RMAT_a, RMAT_b, RMAT_c, 0, directedGraph);
		std::sort( squares.begin(), squares.end() );
	}

	std::vector< std::vector<Square> > rectagnleVecs;
	unsigned long long colIdx = 0;
	unsigned long long rec_x_start = 0;
	auto iter = squares.begin();
	for( auto& rec: squares) {
		if( rec.get_X_start() != rec_x_start ) {
			rectagnleVecs.push_back( std::vector<Square>( iter, iter+colIdx ) );
			iter += colIdx;
			colIdx = 1;
			rec_x_start = rec.get_X_start();
		}
		else {
			++colIdx;
		}
	}
	rectagnleVecs.push_back( std::vector<Square>( iter, iter+colIdx ) );

	if( SHOW_SQUARES_DETAILS )
		for( auto& r: rectagnleVecs )
			for( auto& s: r )
				std::cout << s;

	std::cout << rectagnleVecs.size() << " rectangle(s) specified.\n" << "Generating the graph ...\n";

	// Each thread pushes generated edges into a vector.
	std::vector< std::vector<Edge> > threads_edges(nCPUWorkerThreads);

	if( USE_FUTURES_INSTEAD_OF_EXPLICIT_THREADS ) {

		/*********************************
		 * Multi-threading using futures.
		 *********************************/

		std::vector< std::future<void> > tasks_to_complete;
		// First each PU gets assigned to a job.
		unsigned int recIdx = 0;
		for( ; recIdx < nCPUWorkerThreads && recIdx < rectagnleVecs.size(); ++recIdx )
			tasks_to_complete.push_back( std::async( std::launch::async, EachThreadGeneratesEdges, std::ref(rectagnleVecs.at(recIdx)), std::ref(threads_edges[recIdx]), RMAT_a, RMAT_b, RMAT_c, allowEdgeToSelf, allowDuplicateEdges, directedGraph ) );

		// If any jobs left, main thread has to wait for the first worker thread and write the outcome before initiating any other worker thread.
		for( ; recIdx < rectagnleVecs.size(); ++recIdx ) {
			tasks_to_complete.at(0).get();
			printEdgeGroup(std::ref(threads_edges[recIdx%nCPUWorkerThreads]), outFile);
			progressBar();
			tasks_to_complete.erase(tasks_to_complete.begin());
			tasks_to_complete.push_back( std::async( std::launch::async, EachThreadGeneratesEdges,
					std::ref(rectagnleVecs.at(recIdx)), std::ref(threads_edges[recIdx%nCPUWorkerThreads]),
					RMAT_a, RMAT_b, RMAT_c, allowEdgeToSelf, allowDuplicateEdges, directedGraph ) );
		}

		// Joining last threads.
		for( auto& task: tasks_to_complete ) {
			task.get();
			printEdgeGroup(std::ref(threads_edges[recIdx%nCPUWorkerThreads]), outFile);
			++recIdx;
			progressBar();
		}

		progressBarNewLine();

	} else {

		/*********************************************
		 * Multi-threading using threads explicitly.
		 *********************************************/

		std::vector<std::thread> threads;

		// First each thread gets assigned to a job.
		unsigned int recIdx = 0;
		for( ; recIdx < nCPUWorkerThreads && recIdx < rectagnleVecs.size(); ++recIdx )
			threads.push_back( std::thread( EachThreadGeneratesEdges,
					std::ref(rectagnleVecs.at(recIdx)),
					std::ref(threads_edges[recIdx]),
					RMAT_a, RMAT_b, RMAT_c, allowEdgeToSelf, allowDuplicateEdges, directedGraph ) );

		// If any jobs left, main thread has to wait for the first worker thread and write the outcome before initiating any other worker thread.
		for( ; recIdx < rectagnleVecs.size(); ++recIdx ) {
			threads.at(0).join();
			printEdgeGroup(std::ref(threads_edges[recIdx%nCPUWorkerThreads]), outFile);
			progressBar();
			threads.erase(threads.begin());
			threads.push_back( std::thread( EachThreadGeneratesEdges,
					std::ref(rectagnleVecs.at(recIdx)), std::ref(threads_edges[recIdx%nCPUWorkerThreads]), RMAT_a, RMAT_b, RMAT_c, allowEdgeToSelf, allowDuplicateEdges, directedGraph ) );
		}

		// Joining last threads.
		for( auto& t: threads ) {
			t.join();
			printEdgeGroup(std::ref(threads_edges[recIdx%nCPUWorkerThreads]), outFile);
			++recIdx;
			progressBar();
		}

		progressBarNewLine();

	}

	return( EXIT_SUCCESS );

}
