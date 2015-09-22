#include "Square.hpp"

bool Eligible_RNG_Rec( Square& rec, const unsigned long long nEdgesThreshold ) {
	return ( rec.getnEdges() <= nEdgesThreshold );
}

Square::Square( const Square &cSource ) {
	X_start = cSource.X_start;
	X_end = cSource.X_end;
	Y_start = cSource.Y_start;
	Y_end = cSource.Y_end;
	nEdgeToGenerate = cSource.nEdgeToGenerate;
	level = cSource.level;
	recIndex_horizontal = cSource.recIndex_horizontal;
	recIndex_vertical = cSource.recIndex_vertical;
}

Square& Square::operator=( const Square &cSource ) {
	X_start = cSource.X_start;
	X_end = cSource.X_end;
	Y_start = cSource.Y_start;
	Y_end = cSource.Y_end;
	nEdgeToGenerate = cSource.nEdgeToGenerate;
	level = cSource.level;
	recIndex_horizontal = cSource.recIndex_horizontal;
	recIndex_vertical = cSource.recIndex_vertical;

	return *this;
}

Square Square::Get_part( const unsigned int part, double a, double b, double c, double a_edge_share, double b_edge_share, double c_edge_share ) {

	auto x_start_i = ( part == 0 || part == 2 ) ? X_start : ((X_start+X_end)>>1) ;
	auto x_end_i   = ( part == 1 || part == 3 ) ? X_end	: ((X_start+X_end)>>1) ;
	auto y_start_i = ( part == 0 || part == 1 ) ? Y_start : ((Y_start+Y_end)>>1) ;
	auto y_end_i   = ( part == 2 || part == 3 ) ? Y_end	: ((Y_start+Y_end)>>1) ;

	auto d_edge_share = 1.0 - a_edge_share - b_edge_share - c_edge_share;

	unsigned long long new_nEdges= 	(part==0) ? a_edge_share * nEdgeToGenerate :
									(part==1) ? b_edge_share * nEdgeToGenerate :
									(part==2) ? c_edge_share * nEdgeToGenerate :
												d_edge_share * nEdgeToGenerate ;
	auto new_level = level+1;
	auto new_index_h = ( recIndex_horizontal << 1 ) +	( ( part == 0 || part == 2 ) ? 0 : 1 );
	auto new_index_v = ( recIndex_vertical << 1 )   +	( ( part == 0 || part == 1 ) ? 0 : 1 );

	return Square( x_start_i, x_end_i, y_start_i, y_end_i, new_nEdges, new_level, new_index_h, new_index_v );
}

unsigned long long Square::getnEdges() {
	return nEdgeToGenerate;
}
void Square::setnEdges(unsigned long long n) {
	nEdgeToGenerate = n;
}

bool operator< (const Square& cR1, const Square& cR2) {

	if( cR1.recIndex_horizontal < cR2.recIndex_horizontal )
		return true;
	if( cR1.recIndex_horizontal > cR2.recIndex_horizontal )
		return false;
	if( cR1.recIndex_vertical < cR2.recIndex_vertical )	// If they're equal horizontally.
		return true;
	else
		return false;

}

std::ostream& operator<<( std::ostream &out, Square &cRec ) {
	out << "x:[" << cRec.X_start << ", " <<  cRec.X_end << "]  " << "y:[" << cRec.Y_start << ", " <<  cRec.Y_end << "]  " <<
			"-index: [" << cRec.recIndex_horizontal << ", " <<  cRec.recIndex_vertical << "]   - nEdge: " << cRec.nEdgeToGenerate << std::endl;
	return out;
}
