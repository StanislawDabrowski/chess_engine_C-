#pragma once

#include "Move.h"


struct ScoredMove
{
	Move move;
	int16_t score;
	constexpr ScoredMove(Move move, int16_t score)
		: move(move), score(score) { }
	constexpr ScoredMove() = default;
};