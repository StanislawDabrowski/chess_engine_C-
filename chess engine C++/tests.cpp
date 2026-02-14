//#include <iostream>
//#include <cassert>
//#include <string>
//#include <cctype>
//#include <windows.h>
//#include "Board.h"
//#include "Move.h"
//#include "PieceType.h"
//#include "StaticEvaluation.h"
//#include "MoveGenerator.h"
//
//
//
//
//int s(const std::string& square) {
//	if (square.length() != 2)
//		return -1; // invalid input
//
//	char file = std::toupper(square[0]); // 'A' to 'H'
//	char rank = square[1]; // '1' to '8'
//
//	if (file < 'A' || file > 'H' || rank < '1' || rank > '8')
//		return -1; // invalid input
//
//	int fileIndex = file - 'A';
//	int rankIndex = rank - '1';
//
//	return rankIndex * 8 + fileIndex; // index from bottom-left
//}

/*
void tests()
{
	assert(s("e2") == 12);
	assert(s("e4") == 28);
	Board board;
	board.initialize_board();
	board.make_move(Move(s("e2"), s("e4"), PAWN));
	assert(board.P[PAWN][0] == 0x000000001000EF00);
	board.make_move(Move(s("e7"), s("e5"), PAWN));
	assert(board.P[PAWN][0] == 0x000000001000EF00);
	assert(board.P[PAWN][1] == 0x00EF001000000000);
	board.undo_move();
	assert(board.P[PAWN][0] == 0x000000001000EF00);
	assert(board.P[PAWN][1] == 0x00FF000000000000);
	board.undo_move();
	assert(board.P[PAWN][0] == 0x000000000000FF00);
	assert(board.P[PAWN][1] == 0x00FF000000000000);
	Board board2;
	board2.initialize_board();
	assert(board.P[PAWN][0] == board2.P[PAWN][0]);
	assert(board.P[PAWN][1] == board2.P[PAWN][1]);
	std::cout << "All tests passed!" << std::endl;
}


void print_score(StaticEvaluation* se)
{
	se->reset_score();
	se->calculate_score(false);
	std::cout << "score: " << se->score << std::endl;
}
void print_pawns_file_count(Board* board)
{
	std::cout << "White pawns on files: ";
	for (int f = 0; f < 8; ++f)
		std::cout << (int)board->number_of_pawns_on_files[0][f] << " ";
	std::cout << std::endl;
	std::cout << "Black pawns on files: ";
	for (int f = 0; f < 8; ++f)
		std::cout << (int)board->number_of_pawns_on_files[1][f] << " ";
	std::cout << std::endl;
}

void make_move_and_print(Board* board, StaticEvaluation* se, Move m)
{
	board->make_move(m);
	print_score(se);
	print_pawns_file_count(board);
	board->display_board();
}


void non_assert_based_tests()
{
	
	Board board;
	StaticEvaluation se(&board);
	MoveGenerator mg(&board);
	board.initialize_board();

	/*
	print_score(&se);
	print_pawns_file_count(&board);
	board.display_board();

	make_move_and_print(&board, &se, Move(s("e2"), s("e4"), PAWN));
	make_move_and_print(&board, &se, Move(s("a7"), s("a5"), PAWN));
	make_move_and_print(&board, &se, Move(s("e4"), s("e5"), PAWN));
	make_move_and_print(&board, &se, Move(s("f7"), s("f5"), PAWN));
	make_move_and_print(&board, &se, Move(s("e5"), s("f6"), PAWN, EMPTY));
	make_move_and_print(&board, &se, Move(s("b7"), s("b6"), PAWN));
	make_move_and_print(&board, &se, Move(s("f6"), s("g7"), PAWN, PAWN));
	make_move_and_print(&board, &se, Move(s("a5"), s("a4"), PAWN));
	make_move_and_print(&board, &se, Move(s("g7"), s("h8"), PAWN, ROOK, QUEEN));
	
	mg.generate_pseudo_legal_moves(board.side_to_move);
	mg.list_pseudo_legal_moves();
	make_move_and_print(&board, &se, Move(s("a4"), s("a3"), PAWN));
	make_move_and_print(&board, &se, Move(s("h8"), s("g8"), QUEEN, KNIGHT));
	make_move_and_print(&board, &se, Move(s("a3"), s("b2"), PAWN, PAWN));
	make_move_and_print(&board, &se, Move(s("g8"), s("f8"), QUEEN, BISHOP));
	make_move_and_print(&board, &se, Move(s("b2"), s("a1"), PAWN, ROOK, QUEEN));
	

	for (int _ = 0; _ < 14; _++)
	{
		board.undo_move();
		print_score(&se);
		print_pawns_file_count(&board);
		board.display_board();
	}
	*//*print_score(&se);
	print_pawns_file_count(&board);
	board.display_board();

	//spanish opening
	make_move_and_print(&board, &se, Move(s("e2"), s("e4"), PAWN));
	make_move_and_print(&board, &se, Move(s("e7"), s("e5"), PAWN));
	make_move_and_print(&board, &se, Move(s("g1"), s("f3"), KNIGHT));
	make_move_and_print(&board, &se, Move(s("b8"), s("c6"), KNIGHT));
	make_move_and_print(&board, &se, Move(s("b1"), s("c3"), KNIGHT));
	make_move_and_print(&board, &se, Move(s("g8"), s("f6"), KNIGHT));
	make_move_and_print(&board, &se, Move(s("f1"), s("b5"), BISHOP));
	make_move_and_print(& board, & se, Move(s("f8"), s("b4"), BISHOP));
	//move pawns ot pin the knights
	make_move_and_print(&board, &se, Move(s("d2"), s("d3"), PAWN));
	make_move_and_print(&board, &se, Move(s("d7"), s("d6"), PAWN));

	make_move_and_print(&board, &se, Move(s("c3"), s("d5"), KNIGHT));
	make_move_and_print(&board, &se, Move(s("c6"), s("d4"), KNIGHT));

	make_move_and_print(&board, &se, Move(s("c2"), s("c3"), PAWN));
	make_move_and_print(&board, &se, Move(s("c7"), s("c6"), PAWN));

	

	mg.generate_pseudo_legal_moves_with_ordering_2();
	std::cout << "pseudo legal movews:\n";
	mg.list_pseudo_legal_moves();
	std::cout << "legal moves:\n";
	mg.filter_pseudo_legal_moves();
	mg.list_legal_moves();

	for (int _ = 0; _ < 14; _++)
	{
		board.undo_move();
		print_score(&se);
		print_pawns_file_count(&board);
		board.display_board();
	}
}

int main2()
{
	ULONG_PTR lowLimit, highLimit;
	GetCurrentThreadStackLimits(&lowLimit, &highLimit);
	std::cout << "Stack low: " << lowLimit << "\n";
	std::cout << "Stack high: " << highLimit << "\n";
	std::cout << "Stack size: " << (highLimit - lowLimit) << " bytes\n";
	tests();
	non_assert_based_tests();
	return 0;
}*/