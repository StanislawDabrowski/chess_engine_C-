#include <chrono>
#include <string>
#include "performance_tests.h"
#include "../chess engine C++/Board.h"
#include "../chess engine C++/Move.h"




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
	board.initial_perft(depth);
	return board.perft_nodes_searched;
}