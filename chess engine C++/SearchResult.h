#pragma once
#include <cstdint>
#include "Move.h"


struct SearchResult
{
	Move best_move;
	int16_t score;

	SearchResult(int16_t score, Move best_move);
	SearchResult(int16_t score);
	SearchResult() = default;
};