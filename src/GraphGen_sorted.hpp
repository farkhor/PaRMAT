#ifndef GRAPH_GEN_SORTED_HPP
#define GRAPH_GEN_SORTED_HPP

#include <fstream>


namespace GraphGen_sorted{

	bool GenerateGraph(
			const unsigned long long nEdges,
			const unsigned long long nVertices,
			const double RMAT_a, double RMAT_b, double RMAT_c,
			const unsigned int nCPUWorkerThreads,
			std::ofstream& outFile,
			const unsigned long long standardCapacity,
			const bool allowEdgeToSelf,
			const bool allowDuplicateEdges,
			const bool directedGraph
			);

};

#endif	// GRAPH_GEN_SORTED_HPP
