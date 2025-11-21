#pragma once
#include <cstdint>
#include "TTEntry.h"
#include "Board.h"
#include "Move.h"
#include "SearchResult.h"
#include "StaticEvaluation.h"


class Engine
{
public:
	const uint32_t tt_size = (1ULL << 30) / sizeof(TTEntry);
	TTEntry* tt;

	//uint64_t zobrist_pieces[2][6][64];//piece type, color, square
	//uint64_t zobrist_castling[4];//white kingside, white queenside, black kingside, black queenside
	//uint64_t zobrist_en_passant[64];//all 64 squares, only squares on ranks 3 and 6, which are 16-23 and 40-47 have actual data

	


	Board* board;
	StaticEvaluation se;


	
	Engine(Board* board);
	~Engine();

	//SearchResult find_best_move(uint8_t max_depth);
	//SearchResult minmax_init(uint8_t depth, int16_t alpha, int16_t beta);//returns score and the best move via address. Called only from find_best_move()
	int16_t minmax(uint8_t depth, int16_t alpha, int16_t beta, uint64_t* zobris_key);


};

