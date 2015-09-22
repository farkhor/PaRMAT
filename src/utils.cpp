#include <cmath> 		// For std::floor.
#include <ctime>		// For std::time.
#include <cstdlib>		// For std::rand.
#include <functional>	// For std::ref.
#include <iostream>		// For std::cout.

#if defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "utils.hpp"
#include "internal_config.hpp"


unsigned long long calculateAvailableRAM( const unsigned long long totalRAM, const double memUse ) {
	return static_cast<unsigned long long>( static_cast<double>(totalRAM)*memUse );
}

size_t getTotalSystemMemory() {
#if defined(_WIN32)
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	return status.ullTotalPhys;
#else
	long pages = sysconf(_SC_PHYS_PAGES);
	long page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
#endif
}

bool Eligible_RNG_Squares( std::vector<Square>& squares, const unsigned long long nEdgesThreshold ) {
	unsigned long long nEdgesInEachColumn = 0;	// Accumulated number of edges from squares having the same horizontal coordination.
	unsigned long long beingExamined_X_end = 0;
	for( auto& rec: squares ) {
		if( beingExamined_X_end == rec.get_X_end() ) {
			nEdgesInEachColumn += rec.getnEdges();
		}
		else {
			nEdgesInEachColumn = rec.getnEdges();
			beingExamined_X_end = rec.get_X_end();
		}
		if( nEdgesInEachColumn > nEdgesThreshold )
			return false;
	}

	return true;
}

unsigned long long Get_N_Columns( std::vector<Square>& squares ) {
	unsigned long long nColumns = 0;
	unsigned long long beingExamined_X_end = 0;
	for( auto& rec: squares )
		if( beingExamined_X_end != rec.get_X_end() ) {
			beingExamined_X_end = rec.get_X_end();
			++nColumns;
		}
	return nColumns;
}

void ShatterSquare( std::vector<Square>& square, const double a, const double b, const double c, const unsigned int index, const bool directedGraph ) {

	std::srand(std::time(0));

	// Noise for selecting a, b, c to cut square.
	auto noise_a = static_cast<double>(std::rand()-(RAND_MAX/2))/(RAND_MAX*100.0);	// Very small random noises. Maybe can be implemented in a better way. Up to one percent for each parameter.
	auto noise_b = static_cast<double>(std::rand()-(RAND_MAX/2))/(RAND_MAX*100.0);
	auto noise_c = static_cast<double>(std::rand()-(RAND_MAX/2))/(RAND_MAX*100.0);

	// Noise for number of edges to be created by each sub-square.
	auto noise_a_edge_share = static_cast<double>(std::rand()-(RAND_MAX/2))/(RAND_MAX*100.0);	// Very small random noises. Maybe can be implemented in a better way. Up to one percent for each parameter.
	auto noise_b_edge_share = static_cast<double>(std::rand()-(RAND_MAX/2))/(RAND_MAX*100.0);
	auto noise_c_edge_share = static_cast<double>(std::rand()-(RAND_MAX/2))/(RAND_MAX*100.0);

	Square srcRect(square.at(index));
	square.erase(square.begin()+index);

	// Create and push 4 resulted square from the shattered one.
	for( unsigned int i = 0; i < 4; ++i )
		square.push_back(srcRect.Get_part( i, noise_a+a, noise_b+b, noise_c+c, noise_a_edge_share+a, noise_b_edge_share+b, noise_c_edge_share+c ));

	// Renormalizing: making sure the number of edges still matches, otherwise multiplications with double may impact final total number of edges.
	square.at(square.size()-1).setnEdges( 	srcRect.getnEdges() -
													square.at(square.size()-2).getnEdges() -
													square.at(square.size()-3).getnEdges() -
													square.at(square.size()-4).getnEdges() );

	if( !directedGraph ) {
		if( srcRect.get_X_end() == srcRect.get_Y_end() ) {	// If the source rectangle is on the main diagonal, we should throw away the 2nd part (b).
			square.at(square.size()-2).setnEdges( 	square.at(square.size()-2).getnEdges() +
															square.at(square.size()-3).getnEdges() );	// We add the edges from b part to c part.
			square.erase(square.end()-3);	// erase the b part from the vector.
		}
	}

}

unsigned long long genEdgeIndex_FP( unsigned long long startIdx_ull, unsigned long long endIdx_ull, const double a, const double b_or_c, std::uniform_int_distribution<>& dis, std::mt19937_64& gen ) {

	double noise_a = 0, noise_b_or_c = 0, cutLine;
	double cutIndex;
	auto startIdx = static_cast<double>(startIdx_ull);
	auto endIdx = static_cast<double>(endIdx_ull);
	while( (endIdx - startIdx) >= 1.0 ) {
	#ifdef ADD_NOISE_TO_RMAT_PARAMETERS_AT_EACH_LEVEL
			noise_a = static_cast<double>(dis(gen)-(dis.max()/2.0))/(dis.max()*500.0);	// Much smaller noise. Maybe can be improved.
			noise_b_or_c = static_cast<double>(dis(gen)-(dis.max()/2.0))/(dis.max()*500.0);	// Much smaller noise. Maybe can be improved.
	#endif
		cutLine = a + noise_a + b_or_c + noise_b_or_c ;
		cutIndex = (endIdx+startIdx)/2.0;
		if( (static_cast<double>(dis(gen))/dis.max()) < cutLine )
			endIdx = cutIndex;
		else
			startIdx = cutIndex;
	}
	return static_cast<long long>( std::floor((startIdx+endIdx)/2.0 + 0.5) );

}

void printEdgeGroupNoFlush( std::vector<Edge>& edges, std::ofstream& outFile ) {
	for( auto edge: edges )
		outFile << edge;
}

void printEdgeGroup( std::vector<Edge>& edges, std::ofstream& outFile ) {
	printEdgeGroupNoFlush( edges, outFile );
	outFile.flush();
}

// Checks if there are squares that have to create lots of edges compared to their size and may never finish.
bool edgeOverflow( std::vector<Square>& sqaures ) {
	for( auto& rec : sqaures)
		if( 3*rec.getnEdges() >= (rec.get_X_end()-rec.get_X_start())*(rec.get_Y_end()-rec.get_Y_start()) )	// 3 is experimental. It tells if the size of square is less than 3 times the number of edges, square shouldn't be shattered.
			return true;

	return false;
}


void generate_edges( Square& squ,
		std::vector<Edge>& edgesVec,
		const double RMAT_a, const double RMAT_b, const double RMAT_c,
		const bool directedGraph,
		const bool allowEdgeToSelf,
		std::uniform_int_distribution<>& dis, std::mt19937_64& gen,
		std::vector<unsigned long long>& duplicate_indices ) {

	auto applyCondition = directedGraph || ( squ.get_H_idx() < squ.get_V_idx() ); // true: if the graph is directed or in case it is undirected, the square belongs to the lower triangle of adjacency matrix. false: the diagonal passes the rectangle and the graph is undirected.
	auto createNewEdges = duplicate_indices.empty();
	unsigned long long nEdgesToGen = createNewEdges ? squ.getnEdges() : duplicate_indices.size();
	for( unsigned long long edgeIdx = 0; edgeIdx < nEdgesToGen; ) {
		unsigned long long h_idx = genEdgeIndex_FP(squ.get_X_start(), squ.get_X_end(), RMAT_a, RMAT_c, std::ref(dis), std::ref(gen));
		unsigned long long v_idx = genEdgeIndex_FP(squ.get_Y_start(), squ.get_Y_end(), RMAT_a, RMAT_b, std::ref(dis), std::ref(gen));
		if( (!applyCondition && h_idx > v_idx) || (!allowEdgeToSelf && h_idx == v_idx ) ) // Short-circuit if it doesn't pass the test.
			continue;
		if( createNewEdges )	// Create new edges.
			edgesVec.push_back(	Edge( h_idx, v_idx ) );
		else	// Replace non-valids.
			edgesVec[duplicate_indices[edgeIdx]] = ( Edge( h_idx, v_idx ) );
		++edgeIdx;
	}

}

void progressBar() {
	if( SHOW_PROGRESS_BARS ) {
		std::cout <<'|';
		std::cout.flush();
	}
}

void progressBarNewLine() {
	if( SHOW_PROGRESS_BARS )
		std::cout << std::endl;
}
