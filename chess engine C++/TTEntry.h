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

	TTEntry() : key(16756826919133931531), score(75), flag(EXACT), depth(0), best_move(1804, QUIET_PAWN) {}//initial evaluation at depth 0 (zobirst key of the initial position and evaluation of initial position, e2e4 as best_move to give a legal move
	TTEntry(uint64_t key, int16_t score, ScoreType flag, uint8_t depth, Move move)
		: key(key), score(score), flag(flag), depth(depth), best_move(move) {}
};

