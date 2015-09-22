#include "Edge.hpp"


Edge::Edge(EdgeIndexType rec_src, EdgeIndexType rec_dst):
		src(rec_src), dst(rec_dst)
{}

Edge::Edge(const Edge &cSource) {
	src = cSource.src;
	dst = cSource.dst;
}

Edge& Edge::operator= (const Edge &cSource) {
	src = cSource.src;
	dst = cSource.dst;
	return *this;
}

bool Edge::selfEdge(){
	return ( src == dst );
}

bool operator< (const Edge& cR1, const Edge& cR2) {
	if( cR1.src < cR2.src )
		return true;
	else if( cR1.src > cR2.src )
		return false;
	else if( cR1.dst < cR2.dst )
		return true;
	else
		return false;
}

bool operator== (const Edge& cR1, const Edge& cR2) {
	return ( cR1.dst == cR2.dst && cR1.src == cR2.src);
}

std::ostream& operator<< (std::ostream &out, Edge &cEdge) {
	out << cEdge.src << "\t" << cEdge.dst << "\n";
	return out;
}
