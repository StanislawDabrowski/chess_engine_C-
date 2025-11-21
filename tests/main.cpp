#include <iostream>
#include "Board.h"
#include "Move.h"
#include "unit_tests.h"
#include "performance_tests.h"
#include <Board.cpp>
#include <string>


SimpleMove create_simple_move(std::string move)
{
	std::string files = "abcdefgh";
	std::string ranks = "12345678";
	int from_file = files.find(move[0]);
	int from_rank = ranks.find(move[1]);
	int to_file = files.find(move[2]);
	int to_rank = ranks.find(move[3]);
	int from_square = from_rank * 8 + from_file;
	int to_square = to_rank * 8 + to_file;
	return (to_square << 6) | from_square;
}


int main()
{
	//run_unit_tests(-1);

	int n = 7;
	//for (int i = 0;i<=n;++i)
		//std::cout<<perft(i)<<std::endl;


	Board board;
	board.initialize_board();
	/*constexpr int moves_count = 5;
	Move moves[moves_count];
	moves[0] = Move(create_simple_move("e2e3"), QUIET_PAWN);
	moves[1] = Move(create_simple_move("a7a6"), QUIET_PAWN);
	moves[2] = Move(create_simple_move("g1f3"), QUIET_KNIGHT);
	moves[3] = Move(create_simple_move("a6a5"), QUIET_PAWN);
	moves[4] = Move(create_simple_move("f1e2"), QUIET_BISHOP);
	//moves[5] = Move(create_simple_move("d7d5"), QUIET_PAWN);
	for (int i = 0; i < moves_count; i++)
	{
		std::cout << move_to_string(moves[i].move) << std::endl;
		board.make_move(moves[i].move, moves[i].move_type);
	}*/
	board.display_board();
	std::cout << "sum: " << board.initial_perft(7, SimpleMove(1<<6 & 1)) << std::endl;


	return 0;
}