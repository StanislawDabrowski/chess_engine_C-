#pragma once
#include <cstdint>
#include "Move.h"
#include "ScoreType.h"


class TTEntry
{
public:
	uint64_t key;
	int16_t score;
	ScoreType flag;
	uint8_t depth;
	Move best_move;

	TTEntry() : key(0ULL-1), score(0), flag(EXACT), depth(0), best_move() {}
	TTEntry(uint64_t key, int16_t score, ScoreType flag, uint8_t depth, Move move)
		: key(key), score(score), flag(flag), depth(depth), best_move(move) {}
};

