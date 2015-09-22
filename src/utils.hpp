#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <random>

#include "Square.hpp"
#include "Edge.hpp"


unsigned long long calculateAvailableRAM( const unsigned long long totalRAM, const double memUse );

size_t getTotalSystemMemory();

bool Eligible_RNG_Squares( std::vector<Square>& squares, const unsigned long long nEdgesThreshold );	// If total number of edges that should be created in every column satisfies the memory condition.

unsigned long long Get_N_Columns( std::vector<Square>& squares );	// Gets total number of consecutive squares in one column of the adjacency matrix. Each column needs to be processed by one thread sequentially.

void ShatterSquare( std::vector<Square>& squares, const double a, const double b, const double c, const unsigned int index, const bool directedGraph );

unsigned long long genEdgeIndex_FP( unsigned long long startIdx, unsigned long long endIdx, const double a, const double b_or_c, std::uniform_int_distribution<>& dis, std::mt19937_64& gen );

void printEdgeGroupNoFlush( std::vector<Edge>& edges, std::ofstream& outFile );
void printEdgeGroup( std::vector<Edge>& edges, std::ofstream& outFile );

bool edgeOverflow( std::vector<Square>& squares );

void generate_edges( Square& squ,
		std::vector<Edge>& edgesVec,
		const double RMAT_a, const double RMAT_b, const double RMAT_c,
		const bool directedGraph,
		const bool allowEdgeToSelf,
		std::uniform_int_distribution<>& dis, std::mt19937_64& gen,
		std::vector<unsigned long long>& duplicate_indices );	// The last vector being empty indicates new edges need to be created. Otherwise, replacement edges instead of non-valid ones get created.

void progressBar();
void progressBarNewLine();

#endif	//	UTILS_HPP
