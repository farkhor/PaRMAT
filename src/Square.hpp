#ifndef SQUARE_HPP
#define SQUARE_HPP

#include <fstream>


class Square{

protected:
	unsigned long long X_start, X_end, Y_start, Y_end;
	unsigned long long nEdgeToGenerate, level, recIndex_horizontal, recIndex_vertical;

public:
	Square(){}
	Square( unsigned long long p_x_start, unsigned long long p_x_end,
				unsigned long long p_y_start, unsigned long long p_y_end,
				unsigned long long p_nedge, unsigned long long p_lev, unsigned long long p_index_h, unsigned long long p_index_v ) :

		X_start ( p_x_start ),
		X_end ( p_x_end ),
		Y_start ( p_y_start ),
		Y_end ( p_y_end ),
		nEdgeToGenerate ( p_nedge ),
		level ( p_lev ),
		recIndex_horizontal ( p_index_h ),
		recIndex_vertical ( p_index_v )

	{}

	// Copy constructor.
	Square( const Square &cSource );

	// Assignment operator.
	Square& operator= ( const Square &cSource );

	friend bool Eligible_RNG_Rec ( Square& rec, const unsigned long long nEdgesThreshold );
	Square Get_part( const unsigned int part, double a, double b, double c, double a_edge_share, double b_edge_share, double c_edge_share );
	unsigned long long getnEdges();
	void setnEdges(unsigned long long n);
	friend bool operator< ( const Square& cR1, const Square& cR2 );

	unsigned long long get_X_start(){ return X_start; };
	unsigned long long get_X_end(){ return X_end; };
	unsigned long long get_Y_start(){ return Y_start; };
	unsigned long long get_Y_end(){ return Y_end; };

	unsigned long long get_H_idx(){ return recIndex_horizontal; };
	unsigned long long get_V_idx(){ return recIndex_vertical; };

	friend std::ostream& operator<< ( std::ostream &out, Square &cRec );

};

#endif	// SQUARE_HPP
