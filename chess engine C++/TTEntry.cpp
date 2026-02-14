#include "TTEntry.h"
#include "MoveType.h"
#include "Move.h"
#include "ScoreType.h"


void TTEntry::set_default()
{
	key = 16756826919133931531;
	score = 75;
	flag = EXACT;
	depth = 0;
	best_move = Move(1804, QUIET_PAWN);
}