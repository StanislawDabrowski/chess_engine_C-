#pragma once
#include <cstdint>
#include "Move.h"


struct SearchResult
{
	int16_t score;
	Move best_move;

	SearchResult(int16_t score, Move best_move)
		: score(score), best_move(best_move) {}
};