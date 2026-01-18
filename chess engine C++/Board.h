#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "MoveRecord.h"
#include "MoveGenerator.h"
#include "StaticEvaluation.h"

typedef uint64_t Bitboard;



class Board
{
private:
	static constexpr uint16_t maximum_moves_in_game = 1 << 14;
	MoveRecord moves_stack[maximum_moves_in_game];
	uint64_t repetition_table[maximum_moves_in_game];

	//hardcoded
	uint64_t zobrist_pieces[2][6][64];

	uint64_t zobrist_castling[4];

	uint64_t zobrist_en_passant[64];

	uint64_t zobrist_side_to_move;

	uint16_t repetition_table_last_relevant_position;
	uint16_t repetition_table_size;

	void initialize_castling_mask();
	void set_piece_on_square(uint8_t square, PieceType piece, uint8_t side);
	bool set_piece_fen(char c, int sq);
public:
	 Bitboard P[6][2];

	 Bitboard all_pieces_types[2];
	 Bitboard all_pieces;


	 uint8_t castling_mask[64];

	uint8_t castling_rights;//bit 0 - white kingsite, bit 1 - white queensite, bit 2 - black kingsite, bit 3 - black queensite

	uint64_t zobrist_key;



	char number_of_pawns_on_files[2][8];//for each side, how many pawns on each file (a-h)

	int side_to_move;//0 - white, 1 - black
	int en_passant_square;
	int halfmove_clock;

	bool draw;

	//std::vector<MoveRecord> moves_stack;
	uint16_t moves_stack_size = 0;


	MoveGenerator mg;

	StaticEvaluation se;

	uint64_t perft_nodes_searched;



	Board();

	void initialize_board();
	//void make_move(Move m);
	void make_move(Move move);
	void make_move(SimpleMove move, MoveType move_type);
	void undo_move();
	void update_all_pieces_bitboards();//sums up all piece bitboards into all_pieces_types and all_pieces
	bool load_fen(const std::string& fen);
	void calculate_zobrist_key();
	//PieceType opp_piece_on_square(int square);
	//bool is_move_legal(const Move& move);
	void perft(int depth);
	void initial_perft(int depth);
	std::string get_fen();//debug only
	void display_board(std::ostream& output=std::cout);//debug only
	//void display_board();//debug only
	void display_board_each_piece_and_side_separately();//debug only
	bool is_move_valid(Move move);//debug onnly. Only checks if the moveing piece is present and if the move it moves too is empty in canse of quiet move or is an pponent piece in case of a normal capture or in case of an enpassant, the square is an enpassant square

};