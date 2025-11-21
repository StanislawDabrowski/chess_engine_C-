#include <chrono>
#include <string>
#include "performance_tests.h"
#include "Board.h"
#include "Move.h"




void run_performance_tests(int iterations)
{
	return;
}

uint64_t perft(const int depth, std::string fen)
{
	Board board;
	if (fen!= "")
	{
		board.load_fen(fen);
	}
	else
	{
		board.initialize_board();
	}
	return board.perft(depth);
}