#include <limits>
#include <bit>
#include <iostream>
#include <string>
#include "Engine.h"
#include "ScoredMove.h"

#include <fstream>//debug only

//debug only
std::ofstream outFile("output.txt"); // open file for writing
//


/*void print_board_to_file(Board* board)
{
	std::cout<<"file write"<<std::endl;
	outFile << "Side to move: " << (board->side_to_move == 0 ? "White" : "Black") << "\n";
	outFile << "En passant square: " << board->en_passant_square << "\n";
	outFile << "Halfmove clock: " << board->halfmove_clock << "\n";

	for (int rank = 7; rank >= 0; rank--)
	{
		outFile << rank + 1 << " "; // rank on the left
		for (int file = 0; file < 8; file++)
		{
			int square = rank * 8 + file;
			Bitboard mask = 1ULL << square;
			char piece_char = '.';
			if (board->P[PAWN][0] & mask) piece_char = 'P';
			else if (board->P[KNIGHT][0] & mask) piece_char = 'N';
			else if (board->P[BISHOP][0] & mask) piece_char = 'B';
			else if (board->P[ROOK][0] & mask) piece_char = 'R';
			else if (board->P[QUEEN][0] & mask) piece_char = 'Q';
			else if (board->P[KING][0] & mask) piece_char = 'K';
			else if (board->P[PAWN][1] & mask) piece_char = 'p';
			else if (board->P[KNIGHT][1] & mask) piece_char = 'n';
			else if (board->P[BISHOP][1] & mask) piece_char = 'b';
			else if (board->P[ROOK][1] & mask) piece_char = 'r';
			else if (board->P[QUEEN][1] & mask) piece_char = 'q';
			else if (board->P[KING][1] & mask) piece_char = 'k';
			outFile << piece_char << " ";
		}
		outFile << "\n";
	}

	// print file letters at bottom
	outFile << "  a b c d e f g h" << "\n";
	outFile << std::endl;
}*/

uint64_t Engine::minmax_calls_count = 0;
uint64_t Engine::quiescence_search_calls_count = 0;


Engine::Engine(Board* board)
	: board(board)
{
	tt = new TTEntry[tt_size];


	uint8_t* index_pointer = &board->mg.legal_moves_indexes.knight_capture;
	//check if indexing with j in minmax is correct
	if ((index_pointer + 0) != &board->mg.legal_moves_indexes.knight_capture) std::abort();
	if ((index_pointer + 1) != &board->mg.legal_moves_indexes.quiet_knight) std::abort();
	if ((index_pointer + 2) != &board->mg.legal_moves_indexes.bishop_capture) std::abort();
	if ((index_pointer + 3) != &board->mg.legal_moves_indexes.quiet_bishop) std::abort();
	if ((index_pointer + 4) != &board->mg.legal_moves_indexes.rook_capture) std::abort();
	if ((index_pointer + 5) != &board->mg.legal_moves_indexes.quiet_rook) std::abort();
	if ((index_pointer + 6) != &board->mg.legal_moves_indexes.queen_capture) std::abort();
	if ((index_pointer + 7) != &board->mg.legal_moves_indexes.quiet_queen) std::abort();
	if ((index_pointer + 8) != &board->mg.legal_moves_indexes.king_capture) std::abort();
	if ((index_pointer + 9) != &board->mg.legal_moves_indexes.quiet_king) std::abort();

	index_pointer = &board->mg.legal_moves_indexes.quiet_pawn;
	//check if indexing with j in quiescence_search is correct
	if ((index_pointer + 0) != &board->mg.legal_moves_indexes.quiet_pawn) std::abort();
	if ((index_pointer + 1) != &board->mg.legal_moves_indexes.pawn_capture) std::abort();
	if ((index_pointer + 2) != &board->mg.legal_moves_indexes.queen_promotion) std::abort();
	if ((index_pointer + 3) != &board->mg.legal_moves_indexes.knight_capture) std::abort();
	if ((index_pointer + 4) != &board->mg.legal_moves_indexes.quiet_knight) std::abort();
	if ((index_pointer + 5) != &board->mg.legal_moves_indexes.bishop_capture) std::abort();
	if ((index_pointer + 6) != &board->mg.legal_moves_indexes.quiet_bishop) std::abort();
	if ((index_pointer + 7) != &board->mg.legal_moves_indexes.rook_capture) std::abort();
	if ((index_pointer + 8) != &board->mg.legal_moves_indexes.quiet_rook) std::abort();
	if ((index_pointer + 9) != &board->mg.legal_moves_indexes.queen_capture) std::abort();
	if ((index_pointer + 10) != &board->mg.legal_moves_indexes.quiet_queen) std::abort();
	if ((index_pointer + 11) != &board->mg.legal_moves_indexes.king_capture) std::abort();
	if ((index_pointer + 12) != &board->mg.legal_moves_indexes.quiet_king) std::abort();
}

Engine::~Engine()
{
	delete[] tt;
}


int16_t Engine::minmax(uint8_t depth, int16_t alpha, int16_t beta, bool force_TT_entry_replacement)//in this function knight's and bishop's values are treated as equal, because the difference is negligible
{
	++minmax_calls_count;
	ScoredMove scored_moves[MoveGenerator::max_legal_moves_count];
	//check transposition table
	uint64_t zobrist_key = board->zobrist_key;
	uint32_t zobrist_index = zobrist_key % tt_size;
	//int16_t eval;
	//int16_t min_eval = std::numeric_limits<int16_t>::max();
	//int16_t max_eval = std::numeric_limits<int16_t>::min();
	int16_t search_result;
	if (tt[zobrist_index].key == zobrist_key)
	{		
		if (tt[zobrist_index].depth >= depth)
		{
			if (tt[zobrist_index].flag == EXACT)
				return tt[zobrist_index].score;
			else if (tt[zobrist_index].flag == ALPHA && tt[zobrist_index].score <= alpha)
				return tt[zobrist_index].score;
			else if (tt[zobrist_index].flag == BETA && tt[zobrist_index].score >= beta)
				return tt[zobrist_index].score;
		}
		//std::cout << std::to_string(tt[zobrist_index].depth) << "\n" << std::to_string(tt[zobrist_index].key) << " " << std::to_string(zobrist_key)<<std::endl;
		if (depth != 0 && tt[zobrist_index].depth!=0)
		{
			//make the best move from the TT
			if (board->side_to_move)
			{
				
				 
				
				
				board->make_move(tt[zobrist_index].best_move);
				search_result = minmax(depth - 1, alpha, beta, true);
				
				board->undo_move();
				
				
				if (search_result <= alpha)
				{
					if (tt[zobrist_index].depth == depth || force_TT_entry_replacement)
					{
						//save to TT if deapth is larger
						//zobrist_index is already calculated
						//replace the entry
						tt[zobrist_index].depth = depth;
						tt[zobrist_index].score = beta;
						tt[zobrist_index].flag = ALPHA;
						//debug only
						//tt[zobrist_index].best_move = tt[zobrist_index].best_move;
						//tt[zobrist_index].key = zobrist_key;
						//
						//best move and key are unchanged
					}
					return alpha;
				}
				beta = std::min(beta, search_result);
			}
			else
			{
				
				board->make_move(tt[zobrist_index].best_move);
				search_result = minmax(depth - 1, alpha, beta, true);
				board->undo_move();
				if (search_result >= beta)
				{
					//zobrist_index = zobrist_key % tt_size;
					if (tt[zobrist_index].depth == depth || force_TT_entry_replacement)
					{
						//save to TT if deapth is larger
						//zobrist_index is already calculated
						//replace the entry
						tt[zobrist_index].depth = depth;
						tt[zobrist_index].score = alpha;
						tt[zobrist_index].flag = BETA;
						//debug only
						//tt[zobrist_index].best_move = tt[zobrist_index].best_move;
						//tt[zobrist_index].key = zobrist_key;
						//
						//best move and key are unchanged
					}
					return beta;
				}
				alpha = std::max(alpha, search_result);
			}
		}

	}


	if (depth == 0)
	{
		//outFile << "sq b" << std::endl;
		//board->display_board(outFile);
		int16_t score = quiescence_search(alpha, beta);
		//outFile << "sq e" << std::endl;
		if (tt[zobrist_index].depth == 0 || force_TT_entry_replacement)
		{
			//replace the entry
			tt[zobrist_index].depth = 0;
			tt[zobrist_index].key = zobrist_key;
			tt[zobrist_index].score = score;
			tt[zobrist_index].flag = EXACT;
		}
		return score;//board->se.score;
	}
	board->mg.generate_pseudo_legal_moves_with_category_ordering();
	board->mg.filter_pseudo_legal_moves();


	Bitboard squares_attacked_by_piece[6] = {};

	Bitboard defended_squares;
	Bitboard squares_defended_by_pawns;

	Bitboard squares_from_which_piece_can_attack_piece[6][6] = {};

	Bitboard squares_from_which_piece_can_attack[6];

	Bitboard attacked_squares;


	Bitboard piece_copy;

	unsigned long from;

	if (board->side_to_move)//black to move, minimizing player
	{
		/*
		legal moves are stored in a following order:
		pawn quiet
		pawn capture
		pawn promotions
		knight capture
		knight quiet
		bishop capture
		bishop quiet
		rook capture
		rook quiet
		queen capture
		queen quiet
		king capture
		king quiet
		castling
		*/
		
		//white attacks
		


		//defended squares
		//pawns
		squares_defended_by_pawns = ((board->P[PAWN][1] & MoveGenerator::FILE_A_NEGATION) >> 9) | ((board->P[PAWN][1] & MoveGenerator::FILE_H_NEGATION) >> 7);
		defended_squares = squares_defended_by_pawns;
		//knights
		piece_copy = board->P[KNIGHT][1];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			 _BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.knight_attack_tables[from];
			piece_copy &= piece_copy - 1;
		}
		//bishops
		piece_copy = board->P[BISHOP][1];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			 _BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//rooks
		piece_copy = board->P[ROOK][1];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			 _BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//queens
		piece_copy = board->P[QUEEN][1];
		__assume(std::popcount(piece_copy) <= 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 9);
			 _BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])]
				| board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//king
		_BitScanForward64(&from, board->P[KING][1]);
		defended_squares |= board->mg.king_attack_tables[from];


		//attacks
		//pawns
		piece_copy = board->P[PAWN][0];
		__assume(std::popcount(piece_copy) <= 8);
		squares_attacked_by_piece[PAWN] = ((board->P[PAWN][0] & MoveGenerator::FILE_A_NEGATION) << 7) | ((board->P[PAWN][0] & MoveGenerator::FILE_H_NEGATION) << 9);
		squares_from_which_piece_can_attack_piece[PAWN][PAWN] = squares_attacked_by_piece[PAWN];
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 8);
			_BitScanForward64(&from, piece_copy);
			squares_from_which_piece_can_attack_piece[KNIGHT][PAWN] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][PAWN] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][PAWN] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][PAWN] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}


		//knights
		piece_copy = board->P[KNIGHT][0];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] = ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_A_NEGATION) << 7) | ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_H_NEGATION) << 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[KNIGHT] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[KNIGHT][KNIGHT] |= squares_attacked_by_piece[KNIGHT];
			squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][KNIGHT] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][KNIGHT] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//bishops
		piece_copy = board->P[BISHOP][0];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][BISHOP] = ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_A_NEGATION) << 7) | ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_H_NEGATION) << 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[BISHOP] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from])*board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			
			squares_from_which_piece_can_attack_piece[KNIGHT][BISHOP] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] |= squares_attacked_by_piece[BISHOP];
			squares_from_which_piece_can_attack_piece[ROOK][BISHOP] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][BISHOP] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//rooks
		piece_copy = board->P[ROOK][0];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][ROOK] = ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_A_NEGATION) << 7) | ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_H_NEGATION) << 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[ROOK] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from])*board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			
			squares_from_which_piece_can_attack_piece[KNIGHT][ROOK] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][ROOK] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][ROOK] |= squares_attacked_by_piece[ROOK];
			squares_from_which_piece_can_attack_piece[KING][ROOK] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//queens
		piece_copy = board->P[QUEEN][0];
		__assume(std::popcount(piece_copy) <= 9);
		squares_from_which_piece_can_attack_piece[PAWN][QUEEN] = ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_A_NEGATION) << 7) | ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_H_NEGATION) << 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 9);
			_BitScanForward64(&from, piece_copy);

			squares_from_which_piece_can_attack_piece[KNIGHT][QUEEN] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][QUEEN] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][QUEEN] |= board->mg.king_attack_tables[from];

			squares_attacked_by_piece[QUEEN] |= squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] | squares_from_which_piece_can_attack_piece[ROOK][QUEEN];

			piece_copy &= piece_copy - 1;
		}

		//king
		__assume(std::popcount(board->P[KING][0]) == 1);
		_BitScanForward64(&from, board->P[KING][0]);
		squares_attacked_by_piece[KING] = board->mg.king_attack_tables[from];
		squares_from_which_piece_can_attack_piece[PAWN][KING] = ((board->P[KING][0] & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_A_NEGATION) << 7) | ((board->P[KING][0] & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_H_NEGATION) << 9);
		squares_from_which_piece_can_attack_piece[KNIGHT][KING] = board->mg.knight_attack_tables[from];
		squares_from_which_piece_can_attack_piece[BISHOP][KING] = board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
		squares_from_which_piece_can_attack_piece[ROOK][KING] = board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
		squares_from_which_piece_can_attack_piece[KING][KING] = squares_attacked_by_piece[KING];


		//set queen attacks
		squares_from_which_piece_can_attack_piece[QUEEN][PAWN] = squares_from_which_piece_can_attack_piece[BISHOP][PAWN] | squares_from_which_piece_can_attack_piece[ROOK][PAWN];
		squares_from_which_piece_can_attack_piece[QUEEN][KNIGHT] = squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] | squares_from_which_piece_can_attack_piece[ROOK][KNIGHT];
		squares_from_which_piece_can_attack_piece[QUEEN][BISHOP] = squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] | squares_from_which_piece_can_attack_piece[ROOK][BISHOP];
		squares_from_which_piece_can_attack_piece[QUEEN][ROOK] = squares_from_which_piece_can_attack_piece[BISHOP][ROOK] | squares_from_which_piece_can_attack_piece[ROOK][ROOK];
		squares_from_which_piece_can_attack_piece[QUEEN][KING] = squares_from_which_piece_can_attack_piece[BISHOP][KING] | squares_from_which_piece_can_attack_piece[ROOK][KING];



		squares_from_which_piece_can_attack[PAWN] = squares_from_which_piece_can_attack_piece[PAWN][PAWN] | squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] | squares_from_which_piece_can_attack_piece[PAWN][BISHOP] | squares_from_which_piece_can_attack_piece[PAWN][ROOK] | squares_from_which_piece_can_attack_piece[PAWN][QUEEN] | squares_from_which_piece_can_attack_piece[PAWN][KING];
		squares_from_which_piece_can_attack[KNIGHT] = squares_from_which_piece_can_attack_piece[KNIGHT][PAWN] | squares_from_which_piece_can_attack_piece[KNIGHT][KNIGHT] | squares_from_which_piece_can_attack_piece[KNIGHT][BISHOP] | squares_from_which_piece_can_attack_piece[KNIGHT][ROOK] | squares_from_which_piece_can_attack_piece[KNIGHT][QUEEN] | squares_from_which_piece_can_attack_piece[KNIGHT][KING];
		squares_from_which_piece_can_attack[BISHOP] = squares_from_which_piece_can_attack_piece[BISHOP][PAWN] | squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] | squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] | squares_from_which_piece_can_attack_piece[BISHOP][ROOK] | squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] | squares_from_which_piece_can_attack_piece[BISHOP][KING];
		squares_from_which_piece_can_attack[ROOK] = squares_from_which_piece_can_attack_piece[ROOK][PAWN] | squares_from_which_piece_can_attack_piece[ROOK][KNIGHT] | squares_from_which_piece_can_attack_piece[ROOK][BISHOP] | squares_from_which_piece_can_attack_piece[ROOK][ROOK] | squares_from_which_piece_can_attack_piece[ROOK][QUEEN] | squares_from_which_piece_can_attack_piece[ROOK][KING];
		squares_from_which_piece_can_attack[QUEEN] = squares_from_which_piece_can_attack_piece[QUEEN][PAWN] | squares_from_which_piece_can_attack_piece[QUEEN][KNIGHT] | squares_from_which_piece_can_attack_piece[QUEEN][BISHOP] | squares_from_which_piece_can_attack_piece[QUEEN][ROOK] | squares_from_which_piece_can_attack_piece[QUEEN][QUEEN] | squares_from_which_piece_can_attack_piece[QUEEN][KING];


		attacked_squares = squares_attacked_by_piece[PAWN] | squares_attacked_by_piece[KNIGHT] | squares_attacked_by_piece[BISHOP] | squares_attacked_by_piece[ROOK] | squares_attacked_by_piece[QUEEN] | squares_attacked_by_piece[KING];
		
		//calulate mask &= ~defended_squares for each piece attacking less strong or egual strong piece (e.g. for knight attacking e.g. pawn or bishop, not for rook attacking e.g. queen)
		//to consider in the future: add seperate defending masks fore each piece type to allow more precise heuristics
		/*squares_from_which_piece_can_attack_piece[KNIGHT][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[BISHOP][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[ROOK][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[ROOK][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[ROOK][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[KNIGHT][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[KNIGHT][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][ROOK] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][ROOK] &= defended_squares;*/


		


		//iterate over legal moves to calcualte it's heuristic score and store them in scored_moves array
		uint8_t i = 0;
		//quiet pawn
		for (; i<((board->mg.legal_moves_indexes.quiet_pawn + 1) & 0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = QUIET_PAWN;
			scored_moves[i].score = 0;
			SimpleMove m = board->mg.legal_moves[i];
			uint8_t to = m >> 6;
			Bitboard to_mask = 1ULL << to;
			if (squares_from_which_piece_can_attack[PAWN] & to_mask)
			{
				if (squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][BISHOP] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][ROOK] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][QUEEN] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][KING] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KING] * capture_thread_multiplier;
				//squares_defended_by_pawns variable is used only for pawns moves. pawn defended only by non-pawn piece is no really defended
				scored_moves[i].score += (((squares_defended_by_pawns >> to) & 1) - ((squares_defended_by_pawns >> (m & 0b111111)) & 1)) * defended_piece_multiplier[PAWN];//if pawns moves from undefended square to defended square +1, if it moves from defended square to undefended square -1, otherwise 0. ten it's multiplied
				
			}
		}
		//pawn captures
		for (; i < ((board->mg.legal_moves_indexes.pawn_capture + 1)&0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = CAPTURE_WITH_PAWN;
			scored_moves[i].score = 0;
			SimpleMove m = board->mg.legal_moves[i];
			uint8_t to = m >> 6;
			Bitboard to_mask = 1ULL << to;

			if (squares_from_which_piece_can_attack[PAWN] & to_mask)
			{
				if (squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][BISHOP] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][ROOK] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][QUEEN] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][KING] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KING] * capture_thread_multiplier;
			}

			if (board->P[PAWN][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[PAWN] * capture_multiplier;
			else if (board->P[KNIGHT][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_multiplier;
			else if (board->P[BISHOP][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_multiplier;
			else if (board->P[ROOK][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_multiplier;
			else if (board->P[QUEEN][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_multiplier;

			scored_moves[i].score += (((squares_defended_by_pawns >> to) & 1) - ((squares_defended_by_pawns >> (m & 0b111111)) & 1)) * defended_piece_multiplier[PAWN];

		}
		//pawn promotions
		for (; i < ((board->mg.legal_moves_indexes.queen_promotion+1)&0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = QUEEN_PROMOTION;
			scored_moves[i].score = (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[PAWN]) * promotion_multiplier;//promotions are always good
			Bitboard to_mask = 1ULL << (board->mg.legal_moves[i] >> 6);
			if (squares_attacked_by_piece[BISHOP] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[BISHOP]) * promotion_multiplier;
			if (squares_attacked_by_piece[ROOK] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[ROOK]) * promotion_multiplier;
			if (squares_attacked_by_piece[KNIGHT] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[KNIGHT]) * promotion_multiplier;
			//no more heuristics for promotions because almost always they will be chacked as almost first anyway, additional heuristic is not needed
		}

		constexpr MoveType move_types_in_order[] = { CAPTURE_WITH_KNIGHT, QUIET_KNIGHT, CAPTURE_WITH_BISHOP, QUIET_BISHOP, CAPTURE_WITH_ROOK, QUIET_ROOK, CAPTURE_WITH_QUEEN, QUIET_QUEEN, CAPTURE_WITH_KING, QUIET_KING };
		uint8_t piece = KNIGHT;
		uint8_t last_idx_plus_1;
		uint8_t* index_pointer = &board->mg.legal_moves_indexes.knight_capture;
		for (int j = 0; j<10; ++j)
		{
			//capture
			last_idx_plus_1 = ((*(index_pointer+j))+1) & 0xFF;
			for (; i < last_idx_plus_1; ++i)
			{
				scored_moves[i].move.move = board->mg.legal_moves[i];
				scored_moves[i].move.move_type = move_types_in_order[j];
				scored_moves[i].score = 0;
				SimpleMove m = board->mg.legal_moves[i];
				uint8_t to = m >> 6;
				Bitboard to_mask = 1ULL << to;
				for (uint8_t attacked_piece = PAWN; attacked_piece < EMPTY; ++attacked_piece)
				{
					if (squares_from_which_piece_can_attack_piece[piece][attacked_piece] & to_mask)
						scored_moves[i].score += StaticEvaluation::piece_values[attacked_piece];
				}
				for (uint8_t captured_piece = PAWN; captured_piece < EMPTY; ++captured_piece)
				{
					if (board->P[captured_piece][0] & to_mask)
					{
						if (to_mask & attacked_squares)
							scored_moves[i].score += ((StaticEvaluation::piece_values[captured_piece]>StaticEvaluation::piece_values[piece]) ? (StaticEvaluation::piece_values[captured_piece] - StaticEvaluation::piece_values[piece]) : 0) * capture_multiplier;
						break;
					}
				}
				scored_moves[i].score += (((defended_squares >> to) & 1) - ((defended_squares >> (m & 0b111111)) & 1)) * defended_piece_multiplier[piece];
			}
			++j;//increment to the next move type (quiet)
			last_idx_plus_1 = ((*(index_pointer + j)) + 1) & 0xFF;
			for (; i < last_idx_plus_1; ++i)
			{
				scored_moves[i].move.move = board->mg.legal_moves[i];
				scored_moves[i].move.move_type = move_types_in_order[j];
				scored_moves[i].score = 0;
				SimpleMove m = board->mg.legal_moves[i];
				uint8_t to = m >> 6;
				Bitboard to_mask = 1ULL << to;
				for (uint8_t attacked_piece = PAWN; attacked_piece < EMPTY; ++attacked_piece)
				{
					if (squares_from_which_piece_can_attack_piece[piece][attacked_piece] & to_mask)
						scored_moves[i].score += StaticEvaluation::piece_values[attacked_piece];
				}
				scored_moves[i].score += (((defended_squares >> to) & 1) - ((defended_squares >> (m & 0b111111)) & 1)) * defended_piece_multiplier[piece];
			}

			++piece;//iterates over pieces in order: KNIGHT, BISHOP, ROOK, QUEEN, KING
		}



		//sort moves by their heuristic score (higher score first)
		//use insertion sort
		for (uint8_t j = 1; j < i; ++j)
		{
			ScoredMove key = scored_moves[j];
			uint8_t k = j - 1;
			while (k != 0xFF && scored_moves[k].score < key.score)
			{
				scored_moves[k + 1] = scored_moves[k];
				--k;
			}
			scored_moves[(k + 1) & 0xFF] = key;
		}



		

		if (i == 0)
		{
			int index;
			Bitboard potential_attacks;
			//check for checkmate
			//if checkmate goto checkmate_black
			_BitScanForward64(&from, board->P[KING][1]);
			if (board->mg.knight_attack_tables[from] & board->P[KNIGHT][0])
				goto checkmate_black;
			if (board->mg.pawn_attack_tables[1][from] & board->P[PAWN][0])
				goto checkmate_black;
			if (board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])] & (board->P[BISHOP][0] | board->P[QUEEN][0]))
				goto checkmate_black;
			if (board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])] & (board->P[ROOK][0] | board->P[QUEEN][0]))
				goto checkmate_black;


			return 0;
		checkmate_black:
			return std::numeric_limits<int16_t>::max() - 255 + depth;
		}
		Move best_move = scored_moves[0].move;
		Move m;
		__assume(i <= 255);
		for (int j = 0; j < i; ++j)
		{
			m = scored_moves[j].move;
			board->make_move(m);
			search_result = minmax(depth - 1, alpha, beta);
			board->undo_move();
			if (search_result <= alpha)
			{
				//alpha cut-off
				//save to TT if deapth is larger
				if (tt[zobrist_index].depth <= depth || force_TT_entry_replacement)
				{
					//replace the entry
					tt[zobrist_index].depth = depth;
					tt[zobrist_index].key = zobrist_key;
					tt[zobrist_index].score = alpha;
					tt[zobrist_index].flag = ALPHA;
					tt[zobrist_index].best_move = best_move;
				}
				return alpha;
			}
			beta = std::min(beta, search_result);
		}
		//save to TT if deapth is larger
		//no cut-off occured
		if (tt[zobrist_index].depth <= depth || force_TT_entry_replacement)
		{
			//replace the entry
			tt[zobrist_index].depth = depth;
			tt[zobrist_index].key = zobrist_key;
			tt[zobrist_index].score = beta;
			tt[zobrist_index].flag = EXACT;
			tt[zobrist_index].best_move = best_move;
		}
		return beta;
	}
	else//white to move, maximizing player
	{
		/*
		legal moves are stored in a following order:
		pawn quiet
		pawn capture
		pawn promotions
		knight capture
		knight quiet
		bishop capture
		bishop quiet
		rook capture
		rook quiet
		queen capture
		queen quiet
		king capture
		king quiet
		castling
		*/

		//white attacks


		//defended squares
		//pawns
		squares_defended_by_pawns = ((board->P[PAWN][0] & MoveGenerator::FILE_A_NEGATION) << 7) | ((board->P[PAWN][0] & MoveGenerator::FILE_H_NEGATION) << 9);
		defended_squares = squares_defended_by_pawns;
		//knights
		piece_copy = board->P[KNIGHT][0];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.knight_attack_tables[from];
			piece_copy &= piece_copy - 1;
		}
		//bishops
		piece_copy = board->P[BISHOP][0];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//rooks
		piece_copy = board->P[ROOK][0];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			 _BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//queens
		piece_copy = board->P[QUEEN][0];
		__assume(std::popcount(piece_copy) <= 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 9);
			 _BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])]
				| board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//king
		 _BitScanForward64(&from, board->P[KING][0]);
		defended_squares |= board->mg.king_attack_tables[from];


		//attacks
		//pawns
		piece_copy = board->P[PAWN][1];
		__assume(std::popcount(piece_copy) <= 8);
		squares_attacked_by_piece[PAWN] = ((board->P[PAWN][1] & MoveGenerator::FILE_A_NEGATION) >> 9) | ((board->P[PAWN][1] & MoveGenerator::FILE_H_NEGATION) >> 7);
		squares_from_which_piece_can_attack_piece[PAWN][PAWN] = squares_attacked_by_piece[PAWN];
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 8);
			 _BitScanForward64(&from, piece_copy);
			squares_from_which_piece_can_attack_piece[KNIGHT][PAWN] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][PAWN] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][PAWN] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][PAWN] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}


		//knights
		piece_copy = board->P[KNIGHT][1];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] = ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_A_NEGATION) >> 9) | ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_H_NEGATION) >> 7);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			 _BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[KNIGHT] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[KNIGHT][KNIGHT] |= squares_attacked_by_piece[KNIGHT];
			squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][KNIGHT] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][KNIGHT] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//bishops
		piece_copy = board->P[BISHOP][1];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][BISHOP] = ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_A_NEGATION) >> 9) | ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_H_NEGATION) >> 7);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			 _BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[BISHOP] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];

			squares_from_which_piece_can_attack_piece[KNIGHT][BISHOP] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] |= squares_attacked_by_piece[BISHOP];
			squares_from_which_piece_can_attack_piece[ROOK][BISHOP] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][BISHOP] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//rooks
		piece_copy = board->P[ROOK][1];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][ROOK] = ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_A_NEGATION) >> 9) | ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_H_NEGATION) >> 7);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			 _BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[ROOK] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];

			squares_from_which_piece_can_attack_piece[KNIGHT][ROOK] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][ROOK] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][ROOK] |= squares_attacked_by_piece[ROOK];
			squares_from_which_piece_can_attack_piece[KING][ROOK] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//queens
		piece_copy = board->P[QUEEN][1];
		__assume(std::popcount(piece_copy) <= 9);
		squares_from_which_piece_can_attack_piece[PAWN][QUEEN] = ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_A_NEGATION) >> 9) | ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_H_NEGATION) >> 7);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 9);
			 _BitScanForward64(&from, piece_copy);

			squares_from_which_piece_can_attack_piece[KNIGHT][QUEEN] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][QUEEN] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][QUEEN] |= board->mg.king_attack_tables[from];

			squares_attacked_by_piece[QUEEN] |= squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] | squares_from_which_piece_can_attack_piece[ROOK][QUEEN];

			piece_copy &= piece_copy - 1;
		}

		//king
		__assume(std::popcount(board->P[KING][1]) == 1);
		 _BitScanForward64(&from, board->P[KING][1]);
		squares_attacked_by_piece[KING] = board->mg.king_attack_tables[from];
		squares_from_which_piece_can_attack_piece[PAWN][KING] = ((board->P[KING][0] & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_A_NEGATION) >> 9) | ((board->P[KING][0] & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_H_NEGATION) >> 7);
		squares_from_which_piece_can_attack_piece[KNIGHT][KING] = board->mg.knight_attack_tables[from];
		squares_from_which_piece_can_attack_piece[BISHOP][KING] = board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
		squares_from_which_piece_can_attack_piece[ROOK][KING] = board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
		squares_from_which_piece_can_attack_piece[KING][KING] = squares_attacked_by_piece[KING];


		//set queen attacks
		squares_from_which_piece_can_attack_piece[QUEEN][PAWN] = squares_from_which_piece_can_attack_piece[BISHOP][PAWN] | squares_from_which_piece_can_attack_piece[ROOK][PAWN];
		squares_from_which_piece_can_attack_piece[QUEEN][KNIGHT] = squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] | squares_from_which_piece_can_attack_piece[ROOK][KNIGHT];
		squares_from_which_piece_can_attack_piece[QUEEN][BISHOP] = squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] | squares_from_which_piece_can_attack_piece[ROOK][BISHOP];
		squares_from_which_piece_can_attack_piece[QUEEN][ROOK] = squares_from_which_piece_can_attack_piece[BISHOP][ROOK] | squares_from_which_piece_can_attack_piece[ROOK][ROOK];
		squares_from_which_piece_can_attack_piece[QUEEN][KING] = squares_from_which_piece_can_attack_piece[BISHOP][KING] | squares_from_which_piece_can_attack_piece[ROOK][KING];




		
		squares_from_which_piece_can_attack[PAWN] = squares_from_which_piece_can_attack_piece[PAWN][PAWN] | squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] | squares_from_which_piece_can_attack_piece[PAWN][BISHOP] | squares_from_which_piece_can_attack_piece[PAWN][ROOK] | squares_from_which_piece_can_attack_piece[PAWN][QUEEN] | squares_from_which_piece_can_attack_piece[PAWN][KING];
		squares_from_which_piece_can_attack[KNIGHT] = squares_from_which_piece_can_attack_piece[KNIGHT][PAWN] | squares_from_which_piece_can_attack_piece[KNIGHT][KNIGHT] | squares_from_which_piece_can_attack_piece[KNIGHT][BISHOP] | squares_from_which_piece_can_attack_piece[KNIGHT][ROOK] | squares_from_which_piece_can_attack_piece[KNIGHT][QUEEN] | squares_from_which_piece_can_attack_piece[KNIGHT][KING];
		squares_from_which_piece_can_attack[BISHOP] = squares_from_which_piece_can_attack_piece[BISHOP][PAWN] | squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] | squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] | squares_from_which_piece_can_attack_piece[BISHOP][ROOK] | squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] | squares_from_which_piece_can_attack_piece[BISHOP][KING];
		squares_from_which_piece_can_attack[ROOK] = squares_from_which_piece_can_attack_piece[ROOK][PAWN] | squares_from_which_piece_can_attack_piece[ROOK][KNIGHT] | squares_from_which_piece_can_attack_piece[ROOK][BISHOP] | squares_from_which_piece_can_attack_piece[ROOK][ROOK] | squares_from_which_piece_can_attack_piece[ROOK][QUEEN] | squares_from_which_piece_can_attack_piece[ROOK][KING];
		squares_from_which_piece_can_attack[QUEEN] = squares_from_which_piece_can_attack_piece[QUEEN][PAWN] | squares_from_which_piece_can_attack_piece[QUEEN][KNIGHT] | squares_from_which_piece_can_attack_piece[QUEEN][BISHOP] | squares_from_which_piece_can_attack_piece[QUEEN][ROOK] | squares_from_which_piece_can_attack_piece[QUEEN][QUEEN] | squares_from_which_piece_can_attack_piece[QUEEN][KING];


		attacked_squares = squares_attacked_by_piece[PAWN] | squares_attacked_by_piece[KNIGHT] | squares_attacked_by_piece[BISHOP] | squares_attacked_by_piece[ROOK] | squares_attacked_by_piece[QUEEN] | squares_attacked_by_piece[KING];


		//calulate mask &= ~defended_squares for each piece attacking less strong or egual strong piece (e.g. for knight attacking e.g. pawn or bishop, not for rook attacking e.g. queen)
		//to consider in the future: add seperate defending masks fore each piece type to allow more precise heuristics
		/*squares_from_which_piece_can_attack_piece[KNIGHT][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[BISHOP][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[ROOK][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[ROOK][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[ROOK][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[KNIGHT][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[KNIGHT][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][ROOK] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][ROOK] &= defended_squares;*/


		


		//iterate over legal moves to calcualte it's heuristic score and store them in scored_moves array
		uint8_t i = 0;
		//quiet pawn
		for (; i < ((board->mg.legal_moves_indexes.quiet_pawn + 1) & 0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = QUIET_PAWN;
			scored_moves[i].score = 0;
			SimpleMove m = board->mg.legal_moves[i];
			uint8_t to = m >> 6;
			Bitboard to_mask = 1ULL << to;
			if (squares_from_which_piece_can_attack[PAWN] & to_mask)
			{
				if (squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][BISHOP] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][ROOK] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][QUEEN] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][KING] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KING] * capture_thread_multiplier;
				//squares_defended_by_pawns variable is used only for pawns moves. pawn defended only by non-pawn piece is no really defended
				scored_moves[i].score += (((squares_defended_by_pawns >> to) & 1) - ((squares_defended_by_pawns >> (m & 0b111111)) & 1)) * defended_piece_multiplier[PAWN];//if pawns moves from undefended square to defended square +1, if it moves from defended square to undefended square -1, otherwise 0. ten it's multiplied

			}
		}
		//pawn captures
		for (; i < ((board->mg.legal_moves_indexes.pawn_capture + 1)&0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = CAPTURE_WITH_PAWN;
			scored_moves[i].score = 0;
			SimpleMove m = board->mg.legal_moves[i];
			uint8_t to = m >> 6;
			Bitboard to_mask = 1ULL << to;

			if (squares_from_which_piece_can_attack[PAWN] & to_mask)
			{
				if (squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][BISHOP] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][ROOK] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][QUEEN] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][KING] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KING] * capture_thread_multiplier;
			}

			if (board->P[PAWN][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[PAWN] * capture_multiplier;
			else if (board->P[KNIGHT][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_multiplier;
			else if (board->P[BISHOP][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_multiplier;
			else if (board->P[ROOK][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_multiplier;
			else if (board->P[QUEEN][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_multiplier;

			scored_moves[i].score += (((squares_defended_by_pawns >> to) & 1) - ((squares_defended_by_pawns >> (m & 0b111111)) & 1)) * defended_piece_multiplier[PAWN];

		}
		//pawn promotions
		for (; i < ((board->mg.legal_moves_indexes.queen_promotion + 1)&0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = QUEEN_PROMOTION;
			scored_moves[i].score = (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[PAWN]) * promotion_multiplier;//promotions are always good
			Bitboard to_mask = 1ULL << (board->mg.legal_moves[i] >> 6);
			if (squares_attacked_by_piece[BISHOP] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[BISHOP]) * promotion_multiplier;
			if (squares_attacked_by_piece[ROOK] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[ROOK]) * promotion_multiplier;
			if (squares_attacked_by_piece[KNIGHT] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[KNIGHT]) * promotion_multiplier;
			//no more heuristics for promotions because almost always they will be chacked as almost first anyway, additional heuristic is not needed
		}

		constexpr MoveType move_types_in_order[] = { CAPTURE_WITH_KNIGHT, QUIET_KNIGHT, CAPTURE_WITH_BISHOP, QUIET_BISHOP, CAPTURE_WITH_ROOK, QUIET_ROOK, CAPTURE_WITH_QUEEN, QUIET_QUEEN, CAPTURE_WITH_KING, QUIET_KING };
		uint8_t piece = KNIGHT;
		uint8_t last_idx_plus_1;
		uint8_t* index_pointer = &board->mg.legal_moves_indexes.knight_capture;
		for (int j = 0; j < 10; ++j)
		{
			//capture
			last_idx_plus_1 = ((*(index_pointer + j)) + 1) & 0xFF;
			for (; i < last_idx_plus_1; ++i)
			{
				scored_moves[i].move.move = board->mg.legal_moves[i];
				scored_moves[i].move.move_type = move_types_in_order[j];
				scored_moves[i].score = 0;
				SimpleMove m = board->mg.legal_moves[i];
				uint8_t to = m >> 6;
				Bitboard to_mask = 1ULL << to;
				for (uint8_t attacked_piece = PAWN; attacked_piece < EMPTY; ++attacked_piece)
				{
					if (squares_from_which_piece_can_attack_piece[piece][attacked_piece] & to_mask)
						scored_moves[i].score += StaticEvaluation::piece_values[attacked_piece];
				}
				for (uint8_t captured_piece = PAWN; captured_piece < EMPTY; ++captured_piece)
				{
					if (board->P[captured_piece][0] & to_mask)
					{
						if (to_mask & attacked_squares)
							scored_moves[i].score += ((StaticEvaluation::piece_values[captured_piece] > StaticEvaluation::piece_values[piece]) ? (StaticEvaluation::piece_values[captured_piece] - StaticEvaluation::piece_values[piece]) : 0) * capture_multiplier;
						break;
					}
				}
				scored_moves[i].score += (((defended_squares >> to) & 1) - ((defended_squares >> (m & 0b111111)) & 1)) * defended_piece_multiplier[piece];
			}
			++j;//increment to the next move type (quiet)
			last_idx_plus_1 = ((*(index_pointer + j)) + 1) & 0xFF;//AND to avoid treating 255 as 255 instead of -1
			for (; i < last_idx_plus_1; ++i)
			{
				scored_moves[i].move.move = board->mg.legal_moves[i];
				scored_moves[i].move.move_type = move_types_in_order[j];
				scored_moves[i].score = 0;
				SimpleMove m = board->mg.legal_moves[i];
				uint8_t to = m >> 6;
				Bitboard to_mask = 1ULL << to;
				for (uint8_t attacked_piece = PAWN; attacked_piece < EMPTY; ++attacked_piece)
				{
					if (squares_from_which_piece_can_attack_piece[piece][attacked_piece] & to_mask)
						scored_moves[i].score += StaticEvaluation::piece_values[attacked_piece];
				}
				scored_moves[i].score += (((defended_squares >> to) & 1) - ((defended_squares >> (m & 0b111111)) & 1)) * defended_piece_multiplier[piece];
			}

			++piece;//iterates over pieces in order: KNIGHT, BISHOP, ROOK, QUEEN, KING
		}





		//sort moves by their heuristic score (higher score first)
		//use insertion sort
		for (uint8_t j = 1; j < i; ++j)
		{
			ScoredMove key = scored_moves[j];
			uint8_t k = j - 1;
			while (k != 0xFF && scored_moves[k].score < key.score)
			{
				scored_moves[k + 1] = scored_moves[k];
				--k;
			}
			scored_moves[(k + 1) & 0xFF] = key;
		}





		if (i == 0)
		{
			int index;
			Bitboard potential_attacks;
			//check for checkmate
			//if checkmate goto checkmate_white
			_BitScanForward64(&from, board->P[KING][0]);
			if (board->mg.knight_attack_tables[from] & board->P[KNIGHT][1])
				goto checkmate_white;
			if (board->mg.pawn_attack_tables[0][from] & board->P[PAWN][1])
				goto checkmate_white;
			if (board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])] & (board->P[BISHOP][1] | board->P[QUEEN][1]))
				goto checkmate_white;
			if (board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])] & (board->P[ROOK][1] | board->P[QUEEN][1]))
				goto checkmate_white;


			return 0;
		checkmate_white:
			return std::numeric_limits<int16_t>::min() + 255 - depth;
		}
		Move best_move = scored_moves[0].move;
		Move m;

		for (int j = 0; j < i; ++j)
		{
			m = scored_moves[j].move;
			board->make_move(m);
			search_result = minmax(depth - 1, alpha, beta);
			board->undo_move();
			if (beta <= search_result)
			{
				//beta cut-off
				//save to TT if deapth is larger
				if (tt[zobrist_index].depth <= depth || force_TT_entry_replacement)
				{
					//replace the entry
					tt[zobrist_index].depth = depth;
					tt[zobrist_index].key = zobrist_key;
					tt[zobrist_index].score = alpha;
					tt[zobrist_index].flag = BETA;
					tt[zobrist_index].best_move = best_move;
				}
				return alpha;
			}
			alpha = std::max(alpha, search_result);
		}
		//no cut-off occured
		if (tt[zobrist_index].depth <= depth || force_TT_entry_replacement)
		{
			//replace the entry
			tt[zobrist_index].depth = depth;
			tt[zobrist_index].key = zobrist_key;
			tt[zobrist_index].score = alpha;
			tt[zobrist_index].flag = EXACT;
			tt[zobrist_index].best_move = best_move;
		}
		return alpha;
	}
	
}

SearchResult Engine::minmax_init(uint8_t depth)
{
	int16_t alpha, beta;
	alpha = std::numeric_limits<int16_t>::min();
	beta = std::numeric_limits<int16_t>::max();
	++minmax_calls_count;
	ScoredMove scored_moves[MoveGenerator::max_legal_moves_count];
	//check transposition table
	uint64_t zobrist_key = board->zobrist_key;
	uint32_t zobrist_index = zobrist_key % tt_size;
	//int16_t eval;
	//int16_t min_eval = std::numeric_limits<int16_t>::max();
	//int16_t max_eval = std::numeric_limits<int16_t>::min();
	int16_t search_score;
	if (tt[zobrist_index].key == zobrist_key)
	{
		if (tt[zobrist_index].depth >= depth)
		{
			if (tt[zobrist_index].flag == EXACT)
				return SearchResult(tt[zobrist_index].score, tt[zobrist_index].best_move);
			else if (tt[zobrist_index].flag == ALPHA && tt[zobrist_index].score <= alpha)
				return SearchResult(tt[zobrist_index].score, tt[zobrist_index].best_move);
			else if (tt[zobrist_index].flag == BETA && tt[zobrist_index].score >= beta)
				return SearchResult(tt[zobrist_index].score, tt[zobrist_index].best_move);
		}
		if (depth != 0 && tt[zobrist_index].depth != 0)
		{
			//make the best move from the TT
			if (board->side_to_move)
			{




				board->make_move(tt[zobrist_index].best_move);
				search_score = minmax(depth - 1, alpha, beta, true);

				board->undo_move();

				if (search_score <= alpha)
				{
					if (tt[zobrist_index].depth == depth)
					{
						//save to TT if deapth is larger
						//zobrist_index is already calculated
						//replace the entry
						tt[zobrist_index].depth = depth;
						tt[zobrist_index].score = beta;
						tt[zobrist_index].flag = ALPHA;
						//debug only
						//tt[zobrist_index].best_move = tt[zobrist_index].best_move;
						//tt[zobrist_index].key = zobrist_key;
						//
						//best move and key are unchanged
					}
					return SearchResult(beta, tt[zobrist_index].best_move);
				}
				beta = std::min(beta, search_score);
			}
			else
			{

				board->make_move(tt[zobrist_index].best_move);
				search_score = minmax(depth - 1, alpha, beta, true);
				board->undo_move();
				
				if (beta <= search_score)
				{
					//zobrist_index = zobrist_key % tt_size;
					if (tt[zobrist_index].depth == depth)
					{
						//save to TT if deapth is larger
						//zobrist_index is already calculated
						//replace the entry
						tt[zobrist_index].depth = depth;
						tt[zobrist_index].score = search_score;
						tt[zobrist_index].flag = BETA;
						//debug only
						//tt[zobrist_index].best_move = tt[zobrist_index].best_move;
						//tt[zobrist_index].key = zobrist_key;
						//
						//best move and key are unchanged
					}
					return SearchResult(alpha, tt[zobrist_index].best_move);
				}
				alpha = std::max(alpha, search_score);
			}
		}

	}


	if (depth == 0)
	{
		int16_t score = quiescence_search(alpha, beta);
		if (tt[zobrist_index].depth == 0)
		{
			//replace the entry
			tt[zobrist_index].depth = 0;
			tt[zobrist_index].key = zobrist_key;
			tt[zobrist_index].score = score;
			tt[zobrist_index].flag = EXACT;
		}
		return SearchResult(score, Move());//SearchResult(board->se.score, Move());
	}
	board->mg.generate_pseudo_legal_moves_with_category_ordering();
	board->mg.filter_pseudo_legal_moves();


	Bitboard squares_attacked_by_piece[6] = {};

	Bitboard defended_squares;
	Bitboard squares_defended_by_pawns;

	Bitboard squares_from_which_piece_can_attack_piece[6][6] = {};

	Bitboard squares_from_which_piece_can_attack[6];

	Bitboard attacked_squares;


	Bitboard piece_copy;

	unsigned long from;

	if (board->side_to_move)//black to move, minimizing player
	{
		/*
		legal moves are stored in a following order:
		pawn quiet
		pawn capture
		pawn promotions
		knight capture
		knight quiet
		bishop capture
		bishop quiet
		rook capture
		rook quiet
		queen capture
		queen quiet
		king capture
		king quiet
		castling
		*/

		//white attacks



		//defended squares
		//pawns
		squares_defended_by_pawns = ((board->P[PAWN][1] & MoveGenerator::FILE_A_NEGATION) >> 9) | ((board->P[PAWN][1] & MoveGenerator::FILE_H_NEGATION) >> 7);
		defended_squares = squares_defended_by_pawns;
		//knights
		piece_copy = board->P[KNIGHT][1];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.knight_attack_tables[from];
			piece_copy &= piece_copy - 1;
		}
		//bishops
		piece_copy = board->P[BISHOP][1];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//rooks
		piece_copy = board->P[ROOK][1];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//queens
		piece_copy = board->P[QUEEN][1];
		__assume(std::popcount(piece_copy) <= 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 9);
			_BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])]
				| board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//king
		_BitScanForward64(&from, board->P[KING][1]);
		defended_squares |= board->mg.king_attack_tables[from];


		//attacks
		//pawns
		piece_copy = board->P[PAWN][0];
		__assume(std::popcount(piece_copy) <= 8);
		squares_attacked_by_piece[PAWN] = ((board->P[PAWN][0] & MoveGenerator::FILE_A_NEGATION) << 7) | ((board->P[PAWN][0] & MoveGenerator::FILE_H_NEGATION) << 9);
		squares_from_which_piece_can_attack_piece[PAWN][PAWN] = squares_attacked_by_piece[PAWN];
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 8);
			_BitScanForward64(&from, piece_copy);
			squares_from_which_piece_can_attack_piece[KNIGHT][PAWN] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][PAWN] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][PAWN] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][PAWN] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}


		//knights
		piece_copy = board->P[KNIGHT][0];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] = ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_A_NEGATION) << 7) | ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_H_NEGATION) << 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[KNIGHT] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[KNIGHT][KNIGHT] |= squares_attacked_by_piece[KNIGHT];
			squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][KNIGHT] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][KNIGHT] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//bishops
		piece_copy = board->P[BISHOP][0];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][BISHOP] = ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_A_NEGATION) << 7) | ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_H_NEGATION) << 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[BISHOP] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];

			squares_from_which_piece_can_attack_piece[KNIGHT][BISHOP] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] |= squares_attacked_by_piece[BISHOP];
			squares_from_which_piece_can_attack_piece[ROOK][BISHOP] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][BISHOP] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//rooks
		piece_copy = board->P[ROOK][0];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][ROOK] = ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_A_NEGATION) << 7) | ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_H_NEGATION) << 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[ROOK] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];

			squares_from_which_piece_can_attack_piece[KNIGHT][ROOK] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][ROOK] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][ROOK] |= squares_attacked_by_piece[ROOK];
			squares_from_which_piece_can_attack_piece[KING][ROOK] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//queens
		piece_copy = board->P[QUEEN][0];
		__assume(std::popcount(piece_copy) <= 9);
		squares_from_which_piece_can_attack_piece[PAWN][QUEEN] = ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_A_NEGATION) << 7) | ((piece_copy & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_H_NEGATION) << 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 9);
			_BitScanForward64(&from, piece_copy);

			squares_from_which_piece_can_attack_piece[KNIGHT][QUEEN] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][QUEEN] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][QUEEN] |= board->mg.king_attack_tables[from];

			squares_attacked_by_piece[QUEEN] |= squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] | squares_from_which_piece_can_attack_piece[ROOK][QUEEN];

			piece_copy &= piece_copy - 1;
		}

		//king
		__assume(std::popcount(board->P[KING][0]) == 1);
		_BitScanForward64(&from, board->P[KING][0]);
		squares_attacked_by_piece[KING] = board->mg.king_attack_tables[from];
		squares_from_which_piece_can_attack_piece[PAWN][KING] = ((board->P[KING][0] & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_A_NEGATION) << 7) | ((board->P[KING][0] & MoveGenerator::RANK_8_NEGATION & MoveGenerator::FILE_H_NEGATION) << 9);
		squares_from_which_piece_can_attack_piece[KNIGHT][KING] = board->mg.knight_attack_tables[from];
		squares_from_which_piece_can_attack_piece[BISHOP][KING] = board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
		squares_from_which_piece_can_attack_piece[ROOK][KING] = board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
		squares_from_which_piece_can_attack_piece[KING][KING] = squares_attacked_by_piece[KING];


		//set queen attacks
		squares_from_which_piece_can_attack_piece[QUEEN][PAWN] = squares_from_which_piece_can_attack_piece[BISHOP][PAWN] | squares_from_which_piece_can_attack_piece[ROOK][PAWN];
		squares_from_which_piece_can_attack_piece[QUEEN][KNIGHT] = squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] | squares_from_which_piece_can_attack_piece[ROOK][KNIGHT];
		squares_from_which_piece_can_attack_piece[QUEEN][BISHOP] = squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] | squares_from_which_piece_can_attack_piece[ROOK][BISHOP];
		squares_from_which_piece_can_attack_piece[QUEEN][ROOK] = squares_from_which_piece_can_attack_piece[BISHOP][ROOK] | squares_from_which_piece_can_attack_piece[ROOK][ROOK];
		squares_from_which_piece_can_attack_piece[QUEEN][KING] = squares_from_which_piece_can_attack_piece[BISHOP][KING] | squares_from_which_piece_can_attack_piece[ROOK][KING];



		squares_from_which_piece_can_attack[PAWN] = squares_from_which_piece_can_attack_piece[PAWN][PAWN] | squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] | squares_from_which_piece_can_attack_piece[PAWN][BISHOP] | squares_from_which_piece_can_attack_piece[PAWN][ROOK] | squares_from_which_piece_can_attack_piece[PAWN][QUEEN] | squares_from_which_piece_can_attack_piece[PAWN][KING];
		squares_from_which_piece_can_attack[KNIGHT] = squares_from_which_piece_can_attack_piece[KNIGHT][PAWN] | squares_from_which_piece_can_attack_piece[KNIGHT][KNIGHT] | squares_from_which_piece_can_attack_piece[KNIGHT][BISHOP] | squares_from_which_piece_can_attack_piece[KNIGHT][ROOK] | squares_from_which_piece_can_attack_piece[KNIGHT][QUEEN] | squares_from_which_piece_can_attack_piece[KNIGHT][KING];
		squares_from_which_piece_can_attack[BISHOP] = squares_from_which_piece_can_attack_piece[BISHOP][PAWN] | squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] | squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] | squares_from_which_piece_can_attack_piece[BISHOP][ROOK] | squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] | squares_from_which_piece_can_attack_piece[BISHOP][KING];
		squares_from_which_piece_can_attack[ROOK] = squares_from_which_piece_can_attack_piece[ROOK][PAWN] | squares_from_which_piece_can_attack_piece[ROOK][KNIGHT] | squares_from_which_piece_can_attack_piece[ROOK][BISHOP] | squares_from_which_piece_can_attack_piece[ROOK][ROOK] | squares_from_which_piece_can_attack_piece[ROOK][QUEEN] | squares_from_which_piece_can_attack_piece[ROOK][KING];
		squares_from_which_piece_can_attack[QUEEN] = squares_from_which_piece_can_attack_piece[QUEEN][PAWN] | squares_from_which_piece_can_attack_piece[QUEEN][KNIGHT] | squares_from_which_piece_can_attack_piece[QUEEN][BISHOP] | squares_from_which_piece_can_attack_piece[QUEEN][ROOK] | squares_from_which_piece_can_attack_piece[QUEEN][QUEEN] | squares_from_which_piece_can_attack_piece[QUEEN][KING];


		attacked_squares = squares_attacked_by_piece[PAWN] | squares_attacked_by_piece[KNIGHT] | squares_attacked_by_piece[BISHOP] | squares_attacked_by_piece[ROOK] | squares_attacked_by_piece[QUEEN] | squares_attacked_by_piece[KING];

		//calulate mask &= ~defended_squares for each piece attacking less strong or egual strong piece (e.g. for knight attacking e.g. pawn or bishop, not for rook attacking e.g. queen)
		//to consider in the future: add seperate defending masks fore each piece type to allow more precise heuristics
		/*squares_from_which_piece_can_attack_piece[KNIGHT][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[BISHOP][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[ROOK][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][PAWN] &= defended_squares;
		squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[ROOK][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[ROOK][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[KNIGHT][BISHOP] &= defended_squares;
		squares_from_which_piece_can_attack_piece[KNIGHT][KNIGHT] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][ROOK] &= defended_squares;
		squares_from_which_piece_can_attack_piece[QUEEN][ROOK] &= defended_squares;*/





		//iterate over legal moves to calcualte it's heuristic score and store them in scored_moves array
		uint8_t i = 0;
		//quiet pawn
		for (; i < ((board->mg.legal_moves_indexes.quiet_pawn + 1) & 0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = QUIET_PAWN;
			scored_moves[i].score = 0;
			SimpleMove m = board->mg.legal_moves[i];
			uint8_t to = m >> 6;
			Bitboard to_mask = 1ULL << to;
			if (squares_from_which_piece_can_attack[PAWN] & to_mask)
			{
				if (squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][BISHOP] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][ROOK] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][QUEEN] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][KING] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KING] * capture_thread_multiplier;
				//squares_defended_by_pawns variable is used only for pawns moves. pawn defended only by non-pawn piece is no really defended
				scored_moves[i].score += (((squares_defended_by_pawns >> to) & 1) - ((squares_defended_by_pawns >> (m & 0b111111)) & 1)) * defended_piece_multiplier[PAWN];//if pawns moves from undefended square to defended square +1, if it moves from defended square to undefended square -1, otherwise 0. ten it's multiplied

			}
		}
		//pawn captures
		for (; i < ((board->mg.legal_moves_indexes.pawn_capture + 1) & 0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = CAPTURE_WITH_PAWN;
			scored_moves[i].score = 0;
			SimpleMove m = board->mg.legal_moves[i];
			uint8_t to = m >> 6;
			Bitboard to_mask = 1ULL << to;

			if (squares_from_which_piece_can_attack[PAWN] & to_mask)
			{
				if (squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][BISHOP] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][ROOK] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][QUEEN] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][KING] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KING] * capture_thread_multiplier;
			}

			if (board->P[PAWN][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[PAWN] * capture_multiplier;
			else if (board->P[KNIGHT][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_multiplier;
			else if (board->P[BISHOP][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_multiplier;
			else if (board->P[ROOK][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_multiplier;
			else if (board->P[QUEEN][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_multiplier;

			scored_moves[i].score += (((squares_defended_by_pawns >> to) & 1) - ((squares_defended_by_pawns >> (m & 0b111111)) & 1)) * defended_piece_multiplier[PAWN];

		}
		//pawn promotions
		for (; i < ((board->mg.legal_moves_indexes.queen_promotion + 1) & 0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = QUEEN_PROMOTION;
			scored_moves[i].score = (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[PAWN]) * promotion_multiplier;//promotions are always good
			Bitboard to_mask = 1ULL << (board->mg.legal_moves[i] >> 6);
			if (squares_attacked_by_piece[BISHOP] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[BISHOP]) * promotion_multiplier;
			if (squares_attacked_by_piece[ROOK] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[ROOK]) * promotion_multiplier;
			if (squares_attacked_by_piece[KNIGHT] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[KNIGHT]) * promotion_multiplier;
			//no more heuristics for promotions because almost always they will be chacked as almost first anyway, additional heuristic is not needed
		}

		static constexpr MoveType move_types_in_order[] = { CAPTURE_WITH_KNIGHT, QUIET_KNIGHT, CAPTURE_WITH_BISHOP, QUIET_BISHOP, CAPTURE_WITH_ROOK, QUIET_ROOK, CAPTURE_WITH_QUEEN, QUIET_QUEEN, CAPTURE_WITH_KING, QUIET_KING };
		uint8_t piece = KNIGHT;
		uint8_t last_idx_plus_1;
		uint8_t* index_pointer = &board->mg.legal_moves_indexes.knight_capture;
		for (int j = 0; j < 10; ++j)
		{
			//capture
			last_idx_plus_1 = ((*(index_pointer + j)) + 1) & 0xFF;
			for (; i < last_idx_plus_1; ++i)
			{
				scored_moves[i].move.move = board->mg.legal_moves[i];
				scored_moves[i].move.move_type = move_types_in_order[j];
				scored_moves[i].score = 0;
				SimpleMove m = board->mg.legal_moves[i];
				uint8_t to = m >> 6;
				Bitboard to_mask = 1ULL << to;
				for (uint8_t attacked_piece = PAWN; attacked_piece < EMPTY; ++attacked_piece)
				{
					if (squares_from_which_piece_can_attack_piece[piece][attacked_piece] & to_mask)
						scored_moves[i].score += StaticEvaluation::piece_values[attacked_piece];
				}
				for (uint8_t captured_piece = PAWN; captured_piece < EMPTY; ++captured_piece)
				{
					if (board->P[captured_piece][0] & to_mask)
					{
						if (to_mask & attacked_squares)
							scored_moves[i].score += ((StaticEvaluation::piece_values[captured_piece] > StaticEvaluation::piece_values[piece]) ? (StaticEvaluation::piece_values[captured_piece] - StaticEvaluation::piece_values[piece]) : 0) * capture_multiplier;
						break;
					}
				}
				scored_moves[i].score += (((defended_squares >> to) & 1) - ((defended_squares >> (m & 0b111111)) & 1)) * defended_piece_multiplier[piece];
			}
			++j;//increment to the next move type (quiet)
			last_idx_plus_1 = ((*(index_pointer + j)) + 1) & 0xFF;
			for (; i < last_idx_plus_1; ++i)
			{
				scored_moves[i].move.move = board->mg.legal_moves[i];
				scored_moves[i].move.move_type = move_types_in_order[j];
				scored_moves[i].score = 0;
				SimpleMove m = board->mg.legal_moves[i];
				uint8_t to = m >> 6;
				Bitboard to_mask = 1ULL << to;
				for (uint8_t attacked_piece = PAWN; attacked_piece < EMPTY; ++attacked_piece)
				{
					if (squares_from_which_piece_can_attack_piece[piece][attacked_piece] & to_mask)
						scored_moves[i].score += StaticEvaluation::piece_values[attacked_piece];
				}
				scored_moves[i].score += (((defended_squares >> to) & 1) - ((defended_squares >> (m & 0b111111)) & 1)) * defended_piece_multiplier[piece];
			}

			++piece;//iterates over pieces in order: KNIGHT, BISHOP, ROOK, QUEEN, KING
		}



		//sort moves by their heuristic score (higher score first)
		//use insertion sort
		for (uint8_t j = 1; j < i; ++j)
		{
			ScoredMove key = scored_moves[j];
			uint8_t k = j - 1;
			while (k != 0xFF && scored_moves[k].score < key.score)
			{
				scored_moves[k + 1] = scored_moves[k];
				--k;
			}
			scored_moves[(k + 1) & 0xFF] = key;
		}





		if (i == 0)
		{
			int index;
			Bitboard potential_attacks;
			//check for checkmate
			//if checkmate goto checkmate_black
			_BitScanForward64(&from, board->P[KING][1]);
			if (board->mg.knight_attack_tables[from] & board->P[KNIGHT][0])
				goto checkmate_black;
			if (board->mg.pawn_attack_tables[1][from] & board->P[PAWN][0])
				goto checkmate_black;
			if (board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])] & (board->P[BISHOP][0] | board->P[QUEEN][0]))
				goto checkmate_black;
			if (board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])] & (board->P[ROOK][0] | board->P[QUEEN][0]))
				goto checkmate_black;


			return SearchResult(0, Move());
		checkmate_black:
			return SearchResult(std::numeric_limits<int16_t>::max() - 255 + depth, Move());
		}
		Move best_move = scored_moves[0].move;
		Move m;
		__assume(i <= 255);
		for (int j = 0; j < i; ++j)
		{
			m = scored_moves[j].move;
			board->make_move(m);
			search_score = minmax(depth - 1, alpha, beta);
			board->undo_move();
			if (search_score <= alpha)
			{
				//alpha cut-off
				//save to TT if deapth is larger
				if (tt[zobrist_index].depth <= depth)
				{
					//replace the entry
					tt[zobrist_index].depth = depth;
					tt[zobrist_index].key = zobrist_key;
					tt[zobrist_index].score = beta;
					tt[zobrist_index].flag = ALPHA;
					tt[zobrist_index].best_move = best_move;
				}
				return SearchResult(beta, best_move);
			}
			beta = std::min(beta, search_score);
		}
		//save to TT if deapth is larger
		//no cut-off occured
		if (tt[zobrist_index].depth <= depth)
		{
			//replace the entry
			tt[zobrist_index].depth = depth;
			tt[zobrist_index].key = zobrist_key;
			tt[zobrist_index].score = beta;
			tt[zobrist_index].flag = EXACT;
			tt[zobrist_index].best_move = best_move;
		}
		return SearchResult(beta, best_move);
	}
	else//white to move, maximizing player
	{
		/*
		legal moves are stored in a following order:
		pawn quiet
		pawn capture
		pawn promotions
		knight capture
		knight quiet
		bishop capture
		bishop quiet
		rook capture
		rook quiet
		queen capture
		queen quiet
		king capture
		king quiet
		castling
		*/

		//white attacks


		//defended squares
		//pawns
		squares_defended_by_pawns = ((board->P[PAWN][0] & MoveGenerator::FILE_A_NEGATION) << 7) | ((board->P[PAWN][0] & MoveGenerator::FILE_H_NEGATION) << 9);
		defended_squares = squares_defended_by_pawns;
		//knights
		piece_copy = board->P[KNIGHT][0];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.knight_attack_tables[from];
			piece_copy &= piece_copy - 1;
		}
		//bishops
		piece_copy = board->P[BISHOP][0];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//rooks
		piece_copy = board->P[ROOK][0];
		__assume(std::popcount(piece_copy) <= 10);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//queens
		piece_copy = board->P[QUEEN][0];
		__assume(std::popcount(piece_copy) <= 9);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 9);
			_BitScanForward64(&from, piece_copy);
			defended_squares |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])]
				| board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			piece_copy &= piece_copy - 1;
		}
		//king
		_BitScanForward64(&from, board->P[KING][0]);
		defended_squares |= board->mg.king_attack_tables[from];


		//attacks
		//pawns
		piece_copy = board->P[PAWN][1];
		__assume(std::popcount(piece_copy) <= 8);
		squares_attacked_by_piece[PAWN] = ((board->P[PAWN][1] & MoveGenerator::FILE_A_NEGATION) >> 9) | ((board->P[PAWN][1] & MoveGenerator::FILE_H_NEGATION) >> 7);
		squares_from_which_piece_can_attack_piece[PAWN][PAWN] = squares_attacked_by_piece[PAWN];
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 8);
			_BitScanForward64(&from, piece_copy);
			squares_from_which_piece_can_attack_piece[KNIGHT][PAWN] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][PAWN] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][PAWN] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][PAWN] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}


		//knights
		piece_copy = board->P[KNIGHT][1];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] = ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_A_NEGATION) >> 9) | ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_H_NEGATION) >> 7);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[KNIGHT] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[KNIGHT][KNIGHT] |= squares_attacked_by_piece[KNIGHT];
			squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][KNIGHT] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][KNIGHT] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//bishops
		piece_copy = board->P[BISHOP][1];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][BISHOP] = ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_A_NEGATION) >> 9) | ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_H_NEGATION) >> 7);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[BISHOP] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];

			squares_from_which_piece_can_attack_piece[KNIGHT][BISHOP] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] |= squares_attacked_by_piece[BISHOP];
			squares_from_which_piece_can_attack_piece[ROOK][BISHOP] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][BISHOP] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//rooks
		piece_copy = board->P[ROOK][1];
		__assume(std::popcount(piece_copy) <= 10);
		squares_from_which_piece_can_attack_piece[PAWN][ROOK] = ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_A_NEGATION) >> 9) | ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_H_NEGATION) >> 7);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 10);
			_BitScanForward64(&from, piece_copy);
			squares_attacked_by_piece[ROOK] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];

			squares_from_which_piece_can_attack_piece[KNIGHT][ROOK] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][ROOK] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][ROOK] |= squares_attacked_by_piece[ROOK];
			squares_from_which_piece_can_attack_piece[KING][ROOK] |= board->mg.king_attack_tables[from];

			piece_copy &= piece_copy - 1;
		}

		//queens
		piece_copy = board->P[QUEEN][1];
		__assume(std::popcount(piece_copy) <= 9);
		squares_from_which_piece_can_attack_piece[PAWN][QUEEN] = ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_A_NEGATION) >> 9) | ((piece_copy & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_H_NEGATION) >> 7);
		while (piece_copy)
		{
			__assume(std::popcount(piece_copy) <= 9);
			_BitScanForward64(&from, piece_copy);

			squares_from_which_piece_can_attack_piece[KNIGHT][QUEEN] |= board->mg.knight_attack_tables[from];
			squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] |= board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[ROOK][QUEEN] |= board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
			squares_from_which_piece_can_attack_piece[KING][QUEEN] |= board->mg.king_attack_tables[from];

			squares_attacked_by_piece[QUEEN] |= squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] | squares_from_which_piece_can_attack_piece[ROOK][QUEEN];

			piece_copy &= piece_copy - 1;
		}

		//king
		__assume(std::popcount(board->P[KING][1]) == 1);
		_BitScanForward64(&from, board->P[KING][1]);
		squares_attacked_by_piece[KING] = board->mg.king_attack_tables[from];
		squares_from_which_piece_can_attack_piece[PAWN][KING] = ((board->P[KING][0] & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_A_NEGATION) >> 9) | ((board->P[KING][0] & MoveGenerator::RANK_1_NEGATION & MoveGenerator::FILE_H_NEGATION) >> 7);
		squares_from_which_piece_can_attack_piece[KNIGHT][KING] = board->mg.knight_attack_tables[from];
		squares_from_which_piece_can_attack_piece[BISHOP][KING] = board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])];
		squares_from_which_piece_can_attack_piece[ROOK][KING] = board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])];
		squares_from_which_piece_can_attack_piece[KING][KING] = squares_attacked_by_piece[KING];


		//set queen attacks
		squares_from_which_piece_can_attack_piece[QUEEN][PAWN] = squares_from_which_piece_can_attack_piece[BISHOP][PAWN] | squares_from_which_piece_can_attack_piece[ROOK][PAWN];
		squares_from_which_piece_can_attack_piece[QUEEN][KNIGHT] = squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] | squares_from_which_piece_can_attack_piece[ROOK][KNIGHT];
		squares_from_which_piece_can_attack_piece[QUEEN][BISHOP] = squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] | squares_from_which_piece_can_attack_piece[ROOK][BISHOP];
		squares_from_which_piece_can_attack_piece[QUEEN][ROOK] = squares_from_which_piece_can_attack_piece[BISHOP][ROOK] | squares_from_which_piece_can_attack_piece[ROOK][ROOK];
		squares_from_which_piece_can_attack_piece[QUEEN][KING] = squares_from_which_piece_can_attack_piece[BISHOP][KING] | squares_from_which_piece_can_attack_piece[ROOK][KING];





		squares_from_which_piece_can_attack[PAWN] = squares_from_which_piece_can_attack_piece[PAWN][PAWN] | squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] | squares_from_which_piece_can_attack_piece[PAWN][BISHOP] | squares_from_which_piece_can_attack_piece[PAWN][ROOK] | squares_from_which_piece_can_attack_piece[PAWN][QUEEN] | squares_from_which_piece_can_attack_piece[PAWN][KING];
		squares_from_which_piece_can_attack[KNIGHT] = squares_from_which_piece_can_attack_piece[KNIGHT][PAWN] | squares_from_which_piece_can_attack_piece[KNIGHT][KNIGHT] | squares_from_which_piece_can_attack_piece[KNIGHT][BISHOP] | squares_from_which_piece_can_attack_piece[KNIGHT][ROOK] | squares_from_which_piece_can_attack_piece[KNIGHT][QUEEN] | squares_from_which_piece_can_attack_piece[KNIGHT][KING];
		squares_from_which_piece_can_attack[BISHOP] = squares_from_which_piece_can_attack_piece[BISHOP][PAWN] | squares_from_which_piece_can_attack_piece[BISHOP][KNIGHT] | squares_from_which_piece_can_attack_piece[BISHOP][BISHOP] | squares_from_which_piece_can_attack_piece[BISHOP][ROOK] | squares_from_which_piece_can_attack_piece[BISHOP][QUEEN] | squares_from_which_piece_can_attack_piece[BISHOP][KING];
		squares_from_which_piece_can_attack[ROOK] = squares_from_which_piece_can_attack_piece[ROOK][PAWN] | squares_from_which_piece_can_attack_piece[ROOK][KNIGHT] | squares_from_which_piece_can_attack_piece[ROOK][BISHOP] | squares_from_which_piece_can_attack_piece[ROOK][ROOK] | squares_from_which_piece_can_attack_piece[ROOK][QUEEN] | squares_from_which_piece_can_attack_piece[ROOK][KING];
		squares_from_which_piece_can_attack[QUEEN] = squares_from_which_piece_can_attack_piece[QUEEN][PAWN] | squares_from_which_piece_can_attack_piece[QUEEN][KNIGHT] | squares_from_which_piece_can_attack_piece[QUEEN][BISHOP] | squares_from_which_piece_can_attack_piece[QUEEN][ROOK] | squares_from_which_piece_can_attack_piece[QUEEN][QUEEN] | squares_from_which_piece_can_attack_piece[QUEEN][KING];


		attacked_squares = squares_attacked_by_piece[PAWN] | squares_attacked_by_piece[KNIGHT] | squares_attacked_by_piece[BISHOP] | squares_attacked_by_piece[ROOK] | squares_attacked_by_piece[QUEEN] | squares_attacked_by_piece[KING];


		
		//iterate over legal moves to calcualte it's heuristic score and store them in scored_moves array
		uint8_t i = 0;
		//quiet pawn
		for (; i < ((board->mg.legal_moves_indexes.quiet_pawn + 1) & 0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = QUIET_PAWN;
			scored_moves[i].score = 0;
			SimpleMove m = board->mg.legal_moves[i];
			uint8_t to = m >> 6;
			Bitboard to_mask = 1ULL << to;
			if (squares_from_which_piece_can_attack[PAWN] & to_mask)
			{
				if (squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][BISHOP] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][ROOK] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][QUEEN] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][KING] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KING] * capture_thread_multiplier;
				//squares_defended_by_pawns variable is used only for pawns moves. pawn defended only by non-pawn piece is no really defended
				scored_moves[i].score += (((squares_defended_by_pawns >> to) & 1) - ((squares_defended_by_pawns >> (m & 0b111111)) & 1)) * defended_piece_multiplier[PAWN];//if pawns moves from undefended square to defended square +1, if it moves from defended square to undefended square -1, otherwise 0. ten it's multiplied

			}
		}
		//pawn captures
		for (; i < ((board->mg.legal_moves_indexes.pawn_capture + 1) & 0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = CAPTURE_WITH_PAWN;
			scored_moves[i].score = 0;
			SimpleMove m = board->mg.legal_moves[i];
			uint8_t to = m >> 6;
			Bitboard to_mask = 1ULL << to;

			if (squares_from_which_piece_can_attack[PAWN] & to_mask)
			{
				if (squares_from_which_piece_can_attack_piece[PAWN][KNIGHT] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][BISHOP] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][ROOK] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][QUEEN] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_thread_multiplier;
				if (squares_from_which_piece_can_attack_piece[PAWN][KING] & to_mask)
					scored_moves[i].score += StaticEvaluation::piece_values[KING] * capture_thread_multiplier;
			}

			if (board->P[PAWN][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[PAWN] * capture_multiplier;
			else if (board->P[KNIGHT][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[KNIGHT] * capture_multiplier;
			else if (board->P[BISHOP][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[BISHOP] * capture_multiplier;
			else if (board->P[ROOK][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[ROOK] * capture_multiplier;
			else if (board->P[QUEEN][0] & to_mask)
				scored_moves[i].score += StaticEvaluation::piece_values[QUEEN] * capture_multiplier;

			scored_moves[i].score += (((squares_defended_by_pawns >> to) & 1) - ((squares_defended_by_pawns >> (m & 0b111111)) & 1)) * defended_piece_multiplier[PAWN];

		}
		//pawn promotions
		for (; i < ((board->mg.legal_moves_indexes.queen_promotion + 1) & 0xFF); ++i)
		{
			scored_moves[i].move.move = board->mg.legal_moves[i];
			scored_moves[i].move.move_type = QUEEN_PROMOTION;
			scored_moves[i].score = (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[PAWN]) * promotion_multiplier;//promotions are always good
			Bitboard to_mask = 1ULL << (board->mg.legal_moves[i] >> 6);
			if (squares_attacked_by_piece[BISHOP] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[BISHOP]) * promotion_multiplier;
			if (squares_attacked_by_piece[ROOK] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[ROOK]) * promotion_multiplier;
			if (squares_attacked_by_piece[KNIGHT] & to_mask)
				scored_moves[i].score -= (StaticEvaluation::piece_values[QUEEN] - StaticEvaluation::piece_values[KNIGHT]) * promotion_multiplier;
			//no more heuristics for promotions because almost always they will be chacked as almost first anyway, additional heuristic is not needed
		}

		static constexpr MoveType move_types_in_order[] = { CAPTURE_WITH_KNIGHT, QUIET_KNIGHT, CAPTURE_WITH_BISHOP, QUIET_BISHOP, CAPTURE_WITH_ROOK, QUIET_ROOK, CAPTURE_WITH_QUEEN, QUIET_QUEEN, CAPTURE_WITH_KING, QUIET_KING };
		uint8_t piece = KNIGHT;
		uint8_t last_idx_plus_1;
		uint8_t* index_pointer = &board->mg.legal_moves_indexes.knight_capture;
		for (int j = 0; j < 10; ++j)
		{
			//capture
			last_idx_plus_1 = ((*(index_pointer + j)) + 1) & 0xFF;
			for (; i < last_idx_plus_1; ++i)
			{
				scored_moves[i].move.move = board->mg.legal_moves[i];
				scored_moves[i].move.move_type = move_types_in_order[j];
				scored_moves[i].score = 0;
				SimpleMove m = board->mg.legal_moves[i];
				uint8_t to = m >> 6;
				Bitboard to_mask = 1ULL << to;
				for (uint8_t attacked_piece = PAWN; attacked_piece < EMPTY; ++attacked_piece)
				{
					if (squares_from_which_piece_can_attack_piece[piece][attacked_piece] & to_mask)
						scored_moves[i].score += StaticEvaluation::piece_values[attacked_piece];
				}
				for (uint8_t captured_piece = PAWN; captured_piece < EMPTY; ++captured_piece)
				{
					if (board->P[captured_piece][0] & to_mask)
					{
						if (to_mask & attacked_squares)
							scored_moves[i].score += ((StaticEvaluation::piece_values[captured_piece] > StaticEvaluation::piece_values[piece]) ? (StaticEvaluation::piece_values[captured_piece] - StaticEvaluation::piece_values[piece]) : 0) * capture_multiplier;
						break;
					}
				}
				scored_moves[i].score += (((defended_squares >> to) & 1) - ((defended_squares >> (m & 0b111111)) & 1)) * defended_piece_multiplier[piece];
			}
			++j;//increment to the next move type (quiet)
			last_idx_plus_1 = ((*(index_pointer + j)) + 1) & 0xFF;//AND to avoid treating 255 as 255 instead of -1
			for (; i < last_idx_plus_1; ++i)
			{
				scored_moves[i].move.move = board->mg.legal_moves[i];
				scored_moves[i].move.move_type = move_types_in_order[j];
				scored_moves[i].score = 0;
				SimpleMove m = board->mg.legal_moves[i];
				uint8_t to = m >> 6;
				Bitboard to_mask = 1ULL << to;
				for (uint8_t attacked_piece = PAWN; attacked_piece < EMPTY; ++attacked_piece)
				{
					if (squares_from_which_piece_can_attack_piece[piece][attacked_piece] & to_mask)
						scored_moves[i].score += StaticEvaluation::piece_values[attacked_piece];
				}
				scored_moves[i].score += (((defended_squares >> to) & 1) - ((defended_squares >> (m & 0b111111)) & 1)) * defended_piece_multiplier[piece];
			}

			++piece;//iterates over pieces in order: KNIGHT, BISHOP, ROOK, QUEEN, KING
		}





		//sort moves by their heuristic score (higher score first)
		//use insertion sort
		for (uint8_t j = 1; j < i; ++j)
		{
			ScoredMove key = scored_moves[j];
			uint8_t k = j - 1;
			while (k != 0xFF && scored_moves[k].score < key.score)
			{
				scored_moves[k + 1] = scored_moves[k];
				--k;
			}
			scored_moves[(k + 1) & 0xFF] = key;
		}





		if (i == 0)
		{
			int index;
			Bitboard potential_attacks;
			//check for checkmate
			//if checkmate goto checkmate_white
			_BitScanForward64(&from, board->P[KING][0]);
			if (board->mg.knight_attack_tables[from] & board->P[KNIGHT][1])
				goto checkmate_white;
			if (board->mg.pawn_attack_tables[0][from] & board->P[PAWN][1])
				goto checkmate_white;
			if (board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])] & (board->P[BISHOP][1] | board->P[QUEEN][1]))
				goto checkmate_white;
			if (board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])] & (board->P[ROOK][1] | board->P[QUEEN][1]))
				goto checkmate_white;


			return SearchResult(0, Move());
		checkmate_white:
			return SearchResult(std::numeric_limits<int16_t>::min() + 255 - depth, Move());
		}
		Move best_move = scored_moves[0].move;
		Move m;

		for (int j = 0; j < i; ++j)
		{
			m = scored_moves[j].move;
			board->make_move(m);
			search_score = minmax(depth - 1, alpha, beta);
			board->undo_move();
			if (beta <= search_score)
			{
				//beta cut-off
				//save to TT if deapth is larger
				if (tt[zobrist_index].depth <= depth)
				{
					//replace the entry
					tt[zobrist_index].depth = depth;
					tt[zobrist_index].key = zobrist_key;
					tt[zobrist_index].score = alpha;
					tt[zobrist_index].flag = BETA;
					tt[zobrist_index].best_move = best_move;
				}
				return SearchResult(alpha, best_move);
			}
			alpha = std::max(alpha, search_score);
		}
		//no cut-off occured
		if (tt[zobrist_index].depth <= depth)
		{
			//replace the entry
			tt[zobrist_index].depth = depth;
			tt[zobrist_index].key = zobrist_key;
			tt[zobrist_index].score = alpha;
			tt[zobrist_index].flag = EXACT;
			tt[zobrist_index].best_move = best_move;
		}
		return SearchResult(alpha, best_move);
	}

}

int16_t Engine::quiescence_search(int16_t alpha, int16_t beta, bool force_TT_entry_replacement)
{
	++quiescence_search_calls_count;
	uint8_t opp = board->side_to_move ^ 1;
	board->se.calculate_score(false);
	int16_t stand_pat = board->se.score;
	if (stand_pat >= beta)
		return beta;
	if (alpha < stand_pat)
		alpha = stand_pat;

	int16_t search_score;

	//int16_t min_eval = std::numeric_limits<int16_t>::max();
	//int16_t max_eval = std::numeric_limits<int16_t>::min();

	board->mg.generate_pseudo_legal_moves_with_category_ordering();
	board->mg.filter_pseudo_legal_moves();


	uint64_t zobrist_key = board->zobrist_key;
	uint32_t zobrist_index = zobrist_key % tt_size;

	if (tt[zobrist_index].key == zobrist_key)
	{
		if (tt[zobrist_index].flag == EXACT || tt[zobrist_index].flag == QUIESCANCE_EXACT)
			return tt[zobrist_index].score;
		else if ((tt[zobrist_index].flag == ALPHA || tt[zobrist_index].flag == QUIESCANCE_ALPHA) && tt[zobrist_index].score <= alpha)
			return tt[zobrist_index].score;
		else if ((tt[zobrist_index].flag == BETA || tt[zobrist_index].flag == QUIESCANCE_BETA) && tt[zobrist_index].score >= beta)
			return tt[zobrist_index].score;


		

		if (tt[zobrist_index].depth != 0)
		{

			
			Bitboard to_mask = board->mg.to_mask[tt[zobrist_index].best_move.move];
			
			if (board->side_to_move)
			{//black to move
				if (board->mg.in_check)
					goto attacks_checking_skip_black;
				switch (tt[zobrist_index].best_move.move_type)
				{
				case CAPTURE_WITH_PAWN:

					if (board->P[PAWN][0] & to_mask || !(board->all_pieces & to_mask))
						goto no_best_move_searching;
					break;
				case CAPTURE_WITH_KNIGHT:
				case CAPTURE_WITH_BISHOP:
					if ((board->P[ROOK][0] & to_mask) || (board->P[QUEEN][0] & to_mask))
						break;
					goto no_best_move_searching;
				case CAPTURE_WITH_ROOK:
					if ((board->P[QUEEN][0] & to_mask))
						break;
					goto no_best_move_searching;
				case QUEEN_PROMOTION:
					break;
				default:
					goto no_best_move_searching;

				}
			attacks_checking_skip_black:


				board->make_move(tt[zobrist_index].best_move);
				search_score = quiescence_search(alpha, beta, true);

				board->undo_move();

				
				if (search_score <= alpha)
				{
					//zobrist_index = zobrist_key % tt_size;
					if (force_TT_entry_replacement)
					{
						//save to TT if deapth is larger
						//zobrist_index is already calculated
						//replace the entry
						tt[zobrist_index].score = beta;
						tt[zobrist_index].flag = QUIESCANCE_ALPHA;
						//debug only
						//tt[zobrist_index].best_move = tt[zobrist_index].best_move;
						//tt[zobrist_index].key = zobrist_key;
						//
						//best move and key are unchanged
					}
					return beta;
				}
				beta = std::min(beta, search_score);
			}
			else
			{

				if (board->mg.in_check)
					goto attacks_checking_skip_white;
				switch (tt[zobrist_index].best_move.move_type)
				{
				case CAPTURE_WITH_PAWN:

					if (board->P[PAWN][1] & to_mask || !(board->all_pieces & to_mask))
						goto no_best_move_searching;
					break;
				case CAPTURE_WITH_KNIGHT:
				case CAPTURE_WITH_BISHOP:
					if ((board->P[ROOK][1] & to_mask) || (board->P[QUEEN][1] & to_mask))
						break;
					goto no_best_move_searching;
				case CAPTURE_WITH_ROOK:
					if ((board->P[QUEEN][1] & to_mask))
						break;
					goto no_best_move_searching;
				case QUEEN_PROMOTION:
					break;
				default:
					goto no_best_move_searching;

				}
			attacks_checking_skip_white:

				board->make_move(tt[zobrist_index].best_move.move, tt[zobrist_index].best_move.move_type);
				search_score = quiescence_search(alpha, beta, true);
				board->undo_move();
				if (beta <= search_score)
				{
					//zobrist_index = zobrist_key % tt_size;
					if (force_TT_entry_replacement)
					{
						//save to TT if deapth is larger
						//zobrist_index is already calculated
						//replace the entry
						tt[zobrist_index].score = alpha;
						tt[zobrist_index].flag = QUIESCANCE_BETA;
						//debug only
						//tt[zobrist_index].best_move = tt[zobrist_index].best_move;
						//tt[zobrist_index].key = zobrist_key;
						//
						//best move and key are unchanged
					}
					return alpha;
				}
				alpha = std::max(alpha, search_score);
			}
		no_best_move_searching:
		;//necessary for syntax
		}
	}



	


	Bitboard squares_attacked_by_piece[6] = {};

	Bitboard defended_squares;
	Bitboard squares_defended_by_pawns;

	Bitboard squares_from_which_piece_can_attack_piece[6][6] = {};

	Bitboard squares_from_which_piece_can_attack_more_valuable_piece[6];

	Bitboard squares_from_which_piece_can_attack[6];

	Bitboard attacked_squares;


	Bitboard piece_copy;

	unsigned long from;


	SimpleMove legal_moves_copy[MoveGenerator::max_legal_moves_count];//necessary since the function is recursive and the mg is global
	MovesIndexes8bit legal_moves_indexes_copy = board->mg.legal_moves_indexes;
	std::memcpy(legal_moves_copy, board->mg.legal_moves, ((board->mg.legal_moves_indexes.castle + 1)&0xFF)*sizeof(SimpleMove));//castle is the last move type
	


	if (board->side_to_move)//black to move, minimizing player
	{
		__assume(board->side_to_move == 1);
		/*
		legal moves are stored in a following order:
		pawn quiet
		pawn capture
		pawn promotions
		knight capture
		knight quiet
		bishop capture
		bishop quiet
		rook capture
		rook quiet
		queen capture
		queen quiet
		king capture
		king quiet
		castling
		*/

		

		int16_t search_result;
		SimpleMove best_move = legal_moves_copy[0];
		SimpleMove m;

		int i = 0;
		static constexpr MoveType move_types_in_order[] = {QUIET_PAWN, CAPTURE_WITH_PAWN, QUEEN_PROMOTION, CAPTURE_WITH_KNIGHT, QUIET_KNIGHT, CAPTURE_WITH_BISHOP, QUIET_BISHOP, CAPTURE_WITH_ROOK, QUIET_ROOK, CAPTURE_WITH_QUEEN, QUIET_QUEEN, CAPTURE_WITH_KING, QUIET_KING };
		uint8_t last_idx_plus_1;
		uint8_t* index_pointer = &legal_moves_indexes_copy.quiet_pawn;
		for (int j = 0; j < 13; ++j)
		{
			last_idx_plus_1 = ((*(index_pointer + j)) + 1) & 0xFF;
			for (; i < last_idx_plus_1; ++i)
			{
				m = legal_moves_copy[i];
				Bitboard to_mask = board->mg.to_mask[m];
				switch (move_types_in_order[j])
				{
				case CAPTURE_WITH_PAWN:

					if (board->P[PAWN][0] & to_mask || !(board->all_pieces & to_mask))
						continue;
					break;
				case CAPTURE_WITH_KNIGHT:
				case CAPTURE_WITH_BISHOP:
					if ((board->P[ROOK][0] & to_mask) || (board->P[QUEEN][0] & to_mask))
						break;
					continue;
				case CAPTURE_WITH_ROOK:
					if ((board->P[QUEEN][0] & to_mask))
						break;
					continue;
				case QUEEN_PROMOTION:
					break;
				default:
					continue;
				}

				board->make_move(m, move_types_in_order[j]);
				search_result = quiescence_search(alpha, beta, false);
				board->undo_move();
				if (search_result <= alpha)
				{
					//alpha cut-off
					if ((tt[zobrist_index].flag>=QUIESCANCE_EXACT && tt[zobrist_index].flag<=QUIESCANCE_BETA) || force_TT_entry_replacement)
					{
						tt[zobrist_index].key = zobrist_key;
						tt[zobrist_index].score = beta;
						tt[zobrist_index].flag = QUIESCANCE_ALPHA;
						tt[zobrist_index].best_move.move = best_move;
						tt[zobrist_index].best_move.move_type = move_types_in_order[j];
					}
					return beta;
				}
				beta = std::min(beta, search_result);



			}
		}




		if (i == 0)
		{
			int index;
			Bitboard potential_attacks;
			//check for checkmate
			//if checkmate goto checkmate_black
			_BitScanForward64(&from, board->P[KING][1]);
			if (board->mg.knight_attack_tables[from] & board->P[KNIGHT][0])
				goto checkmate_black;
			if (board->mg.pawn_attack_tables[1][from] & board->P[PAWN][0])
				goto checkmate_black;
			if (board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])] & (board->P[BISHOP][0] | board->P[QUEEN][0]))
				goto checkmate_black;
			if (board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])] & (board->P[ROOK][0] | board->P[QUEEN][0]))
				goto checkmate_black;


			return 0;
		checkmate_black:
			return std::numeric_limits<int16_t>::max() - 256;//subtract 256 since maximum depth is 255 so 256 to make chackmate in quiescence worse than any checkmate in normal search
		}
		return stand_pat;


		
	}
	else//white to move, maximizing player
	{
		/*
		legal moves are stored in a following order:
		pawn quiet
		pawn capture
		pawn promotions
		knight capture
		knight quiet
		bishop capture
		bishop quiet
		rook capture
		rook quiet
		queen capture
		queen quiet
		king capture
		king quiet
		castling
		*/



		int16_t search_result;
		SimpleMove best_move = legal_moves_copy[0];
		SimpleMove m;

		int i = 0;
		static constexpr MoveType move_types_in_order[] = { QUIET_PAWN, CAPTURE_WITH_PAWN, QUEEN_PROMOTION, CAPTURE_WITH_KNIGHT, QUIET_KNIGHT, CAPTURE_WITH_BISHOP, QUIET_BISHOP, CAPTURE_WITH_ROOK, QUIET_ROOK, CAPTURE_WITH_QUEEN, QUIET_QUEEN, CAPTURE_WITH_KING, QUIET_KING };
		uint8_t last_idx_plus_1;
		uint8_t* index_pointer = &legal_moves_indexes_copy.quiet_pawn;
		for (int j = 0; j < 13; ++j)
		{
			last_idx_plus_1 = ((*(index_pointer + j)) + 1) & 0xFF;
			for (; i < last_idx_plus_1; ++i)
			{
				m = legal_moves_copy[i];

				Bitboard to_mask = board->mg.to_mask[m];
				switch (move_types_in_order[j])
				{
				case CAPTURE_WITH_PAWN:

					if (board->P[PAWN][1] & to_mask || !(board->all_pieces & to_mask))
						continue;
					break;
				case CAPTURE_WITH_KNIGHT:
				case CAPTURE_WITH_BISHOP:
					if ((board->P[ROOK][1] & to_mask) || (board->P[QUEEN][1] & to_mask))
						break;
					continue;
				case CAPTURE_WITH_ROOK:
					if ((board->P[QUEEN][1] & to_mask))
						break;
					continue;
				case QUEEN_PROMOTION:
					break;
				default:
					continue;
				}

				board->make_move(m, move_types_in_order[j]);
				search_result = quiescence_search(alpha, beta, false);
				board->undo_move();
				if (beta <= search_result)
				{
					//alpha cut-off
					if ((tt[zobrist_index].flag >= QUIESCANCE_EXACT && tt[zobrist_index].flag <= QUIESCANCE_BETA) || force_TT_entry_replacement)
					{
						tt[zobrist_index].key = zobrist_key;
						tt[zobrist_index].score = alpha;
						tt[zobrist_index].flag = QUIESCANCE_BETA;
						tt[zobrist_index].best_move.move = best_move;
						tt[zobrist_index].best_move.move_type = move_types_in_order[j];
					}
					return alpha;
				}
				alpha = std::max(alpha, search_result);



			}
		}




		if (i == 0)
		{
			int index;
			Bitboard potential_attacks;
			//check for checkmate
			//if checkmate goto checkmate_black
			_BitScanForward64(&from, board->P[KING][0]);
			if (board->mg.knight_attack_tables[from] & board->P[KNIGHT][1])
				goto checkmate_white;
			if (board->mg.pawn_attack_tables[0][from] & board->P[PAWN][1])
				goto checkmate_white;
			if (board->mg.bishop_attack_tables[from][uint64_t(((board->all_pieces & board->mg.bishop_relevant_blockers[from]) * board->mg.bishop_magic_numbers[from]) >> board->mg.bishop_relevant_bits_shift[from])] & (board->P[BISHOP][1] | board->P[QUEEN][1]))
				goto checkmate_white;
			if (board->mg.rook_attack_tables[from][uint64_t(((board->all_pieces & board->mg.rook_relevant_blockers[from]) * board->mg.rook_magic_numbers[from]) >> board->mg.rook_relevant_bits_shift[from])] & (board->P[ROOK][1] | board->P[QUEEN][1]))
				goto checkmate_white;


			return 0;
		checkmate_white:
			return std::numeric_limits<int16_t>::max() - 256;//subtract 256 since maximum depth is 255 so 256 to make chackmate in quiescence worse than any checkmate in normal search
		}
		return stand_pat;
	}

}
