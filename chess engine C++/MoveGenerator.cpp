#include <bit>
#include <iostream>
#include <vector>
#include <cassert>
//#include <fstream>//debug only
#include "MoveGenerator.h"
#include "Board.h"
#include "Move.h"

//debug only
//std::ofstream outFile("output.txt"); // open file for writing
//

typedef uint16_t SimpleMove;

MoveGenerator::MoveGenerator(Board* board)
	: board(board)
{
	generate_relevant_blockers();
	generate_attack_tables();
	generate_from_to_masks();

	legal_moves_vector.resize(MoveGenerator::max_legal_moves_count);
	pseudo_legal_moves_vector.resize(MoveGenerator::max_pseudo_legal_moves_count);

	
}

void MoveGenerator::generate_relevant_blockers()
{
	int rook_directions[] = { 8, -8, 1, -1 };
	int bishop_directions[] = { 7, -7, 9, -9 };
	Bitboard rook_mask;
	Bitboard bishop_mask;
	int current_square;
	int cs_mod8;
	int temp;
	for (int square = 0; square < 64; ++square)
	{
		rook_mask = 0;
		bishop_mask = 0;
		// Rook relevant blockers
		for (int direction : rook_directions)
		{
			//current_square = square + direction;
			current_square = square;
			while (true)
			{
				temp = current_square + direction;
				if (temp > 63 || temp < 0) break; // stop if out of bounds
				cs_mod8 = current_square % 8;
				if (cs_mod8 == 0 && direction == -1) break; // stop if we hit left edge going left
				if (cs_mod8 == 7 && direction == 1) break; // stop if we hit right edge going right
				if (current_square > 55 && direction == 8) break; // stop if we hit top edge going up
				if (current_square < 8 && direction == -8) break; // stop if we hit bottom edge going down
				if (current_square !=square)
					rook_mask |= (1ULL << current_square);
				current_square += direction;
			}
		}
		rook_relevant_blockers[square] = rook_mask;
		// Bishop relevant blockers
		for (int direction : bishop_directions)
		{
			//current_square = square + direction;
			current_square = square;
			while (true)
			{
				temp = current_square + direction;
				if (temp > 63 || temp < 0) break; // stop if out of bounds
				cs_mod8 = current_square % 8;
				if (cs_mod8 == 0 && (direction == -9 || direction == 7)) break; // stop if we hit left edge going left
				if (cs_mod8 == 7 && (direction == 9 || direction == -7)) break; // stop if we hit right edge going right
				if (current_square > 55 && (direction == 9 || direction == 7)) break; // stop if we hit top edge going up
				if (current_square < 8 && (direction == -7 || direction == -9)) break; // stop if we hit bottom edge going down
				if (current_square != square)
					bishop_mask |= (1ULL << current_square);
				current_square += direction;
			}
		}
		bishop_relevant_blockers[square] = bishop_mask;

	}
}

uint64_t occupancy_from_index(uint32_t index, uint64_t mask) {
	uint64_t occ = 0;
	uint32_t bit = 0;
	for (int sq = 0; sq < 64; ++sq) {
		if (mask & (1ULL << sq)) {
			if (index & (1U << bit)) occ |= (1ULL << sq);
			++bit;
		}
	}
	return occ;
}


void MoveGenerator::generate_attack_tables()
{
	//int* rook_attack_tables_temp = new int[64 * 4096];
	//
	// int* bishop_attack_tables_temp = new int[64 * 512];

	constexpr int rook_directions[] = { 8, -8, 1, -1 };
	constexpr int bishop_directions[] = { 7, -7, 9, -9 };
	Bitboard rook_mask;
	Bitboard bishop_mask;
	Bitboard knight_mask;
	Bitboard king_mask;
	Bitboard blockers_bishop_mask;
	Bitboard blockers_rook_mask;
	Bitboard occupancy;
	int current_square;
	int cs_mod8;
	memset(bishop_attack_tables, 0, sizeof(bishop_attack_tables));
	memset(rook_attack_tables, 0, sizeof(rook_attack_tables));
	for (int square = 0; square < 64; ++square)
	{
		//rook attacks
		for (int i = 0; i < 1u << std::popcount(rook_relevant_blockers[square]); ++i)
		{
			occupancy = occupancy_from_index(i, rook_relevant_blockers[square]);

			rook_mask = 0;
			blockers_rook_mask = 0;
			for (int direction : rook_directions)
			{
				current_square = square;// + direction;
				while (true)
				{
					cs_mod8 = current_square % 8;
					if (current_square!=square)
						rook_mask |= (1ULL << current_square);
					if (occupancy & (1ULL << current_square))
					{
						blockers_rook_mask |= (1ULL << current_square);//add a bolcker
						break; // stop if we hit a blocker
					}
					if (cs_mod8 == 0 && direction == -1) break; // stop if we hit left edge going left
					if (cs_mod8 == 7 && direction == 1) break; // stop if we hit right edge going right
					if (current_square > 55 && direction == 8) break; // stop if we hit top edge going up
					if (current_square < 8 && direction == -8) break; // stop if we hit bottom edge going down

					current_square += direction;
					if (current_square > 63 || current_square < 0) break; // stop if out of bounds
				}
			}

			
			//get with magic numbers index to save the attack table to
			Bitboard relevant_blockers = occupancy & rook_relevant_blockers[square];
			uint32_t index = uint32_t((relevant_blockers * rook_magic_numbers[square]) >> (64 - std::popcount(rook_relevant_blockers[square])));

			if (rook_attack_tables[square][index] != 0 && rook_attack_tables[square][index] != rook_mask) {
				std::cout << "Rook collision: square=" << square
					<< " index=" << index
					<< " existing=" << rook_attack_tables[square][index]
					<< " new=" << rook_mask
					<< " occ=" << occupancy
					<< " popcount=" << std::popcount(rook_relevant_blockers[square])
					<< std::endl;
			}
			rook_attack_tables[square][index] = rook_mask;
			rook_blockers[square][index] = blockers_rook_mask;

		}

		//bishop attacks
		for (int i = 0; i < 1u << std::popcount(bishop_relevant_blockers[square]); ++i)
		{
			occupancy = occupancy_from_index(i, bishop_relevant_blockers[square]);
			bishop_mask = 0;
			blockers_bishop_mask = 0;
			for (int direction : bishop_directions)
			{
				int current_square = square;// + direction;
				while (true)
				{
					cs_mod8 = current_square % 8;
					if (current_square < 0 || current_square >= 64) break; // stop if out of bounds
					if (current_square != square)
						bishop_mask |= (1ULL << current_square);
					if (occupancy & (1ULL << current_square))
					{
						blockers_bishop_mask |= (1ULL << current_square);//add a bolcker
						break; // stop if we hit a blocker
					}
					if (cs_mod8 == 0 && (direction == -9 || direction == 7)) break; // stop if we hit left edge going left
					if (cs_mod8 == 7 && (direction == 9 || direction == -7)) break; // stop if we hit right edge going right

					current_square += direction;
				}
			}
			//get with magic numbers index to save the attack table to
			
			Bitboard relevant_blockers = occupancy & bishop_relevant_blockers[square];
			uint32_t index = uint32_t((relevant_blockers * bishop_magic_numbers[square]) >> (64 - std::popcount(bishop_relevant_blockers[square])));
			

			if (bishop_attack_tables[square][index] != 0 && bishop_attack_tables[square][index] != bishop_mask) {
				std::cout << "Bishop collision: square=" << square
					<< " index=" << index
					<< " existing=" << bishop_attack_tables[square][index]
					<< " new=" << bishop_mask
					<< " occ=" << occupancy
					<< " popcount=" << std::popcount(bishop_relevant_blockers[square])
					<< std::endl;
			}
			bishop_attack_tables[square][index] = bishop_mask;
			bishop_blockers[square][index] = blockers_bishop_mask;


		}
		//knight attacks
		knight_mask = 0;
		int rank = square / 8;
		int file = square % 8;
		int knight_moves[8][2] = { {2,1}, {1,2}, {-1,2}, {-2,1}, {-2,-1}, {-1,-2}, {1,-2}, {2,-1} };
		for (auto& move : knight_moves)
		{
			int new_rank = rank + move[0];
			int new_file = file + move[1];
			if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8)
			{
				knight_mask |= (1ULL << (new_rank * 8 + new_file));
			}
		}
		knight_attack_tables[square] = knight_mask;

		//king attacks
		king_mask = 0;
		int king_moves[8][2] = { {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1} };
		for (auto& move : king_moves)
		{
			int new_rank = rank + move[0];
			int new_file = file + move[1];
			if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8)
			{
				king_mask |= (1ULL << (new_rank * 8 + new_file));
			}
		}
		king_attack_tables[square] = king_mask;
	}
	//pawns
	for (int sq = 0; sq < 64; ++sq)
	{
		pawn_attack_tables[0][sq] = 0;
		pawn_attack_tables[1][sq] = 0;
		int file = sq % 8;
		//white
		if (sq <= 48)
		{
			if (file > 0)
				pawn_attack_tables[0][sq] |= (1ULL << (sq + 7));
			if (file < 7)
				pawn_attack_tables[0][sq] |= (1ULL << (sq + 9));
		}
		//black
		if (sq >= 15)
		{
			if (file > 0)
				pawn_attack_tables[1][sq] |= (1ULL << (sq - 9));
			if (file < 7)
				pawn_attack_tables[1][sq] |= (1ULL << (sq - 7));
		}
	}

	//relevant bits shift
	for (int square = 0; square < 64; ++square)
	{
		bishop_relevant_bits_shift[square] = 64 - std::popcount(bishop_relevant_blockers[square]);
		rook_relevant_bits_shift[square] = 64 - std::popcount(rook_relevant_blockers[square]);
	}


	//castles
	//knights
	
	//white kingside
	Bitboard knight_from_e1 = knight_attack_tables[4];
	Bitboard knight_from_f1 = knight_attack_tables[5];
	Bitboard knight_from_g1 = knight_attack_tables[6];
	white_kingside_castle_knight_attack_mask = knight_from_e1 | knight_from_f1 | knight_from_g1;

	//white queenside
	Bitboard knight_from_d1 = knight_attack_tables[3];
	Bitboard knight_from_c1 = knight_attack_tables[2];
	white_queenside_castle_knight_attack_mask = knight_from_e1 | knight_from_d1 | knight_from_c1;

	//black kingside
	Bitboard knight_from_e8 = knight_attack_tables[60];
	Bitboard knight_from_f8 = knight_attack_tables[61];
	Bitboard knight_from_g8 = knight_attack_tables[62];
	black_kingside_castle_knight_attack_mask = knight_from_e8 | knight_from_f8 | knight_from_g8;

	//black queenside
	Bitboard knight_from_d8 = knight_attack_tables[59];
	Bitboard knight_from_c8 = knight_attack_tables[58];
	black_queenside_castle_knight_attack_mask = knight_from_e8 | knight_from_d8 | knight_from_c8;


	//pawns
	white_kingside_castle_pawn_attack_mask = 0b11111ULL << 11;
	white_queenside_castle_pawn_attack_mask = 0b11111ULL << 9;
	black_kingside_castle_pawn_attack_mask = 0b11111ULL << 51;
	black_queenside_castle_pawn_attack_mask = 0b11111ULL << 49;

	//king
	white_kingside_castle_king_attack_mask = king_attack_tables[4] | king_attack_tables[5] | king_attack_tables[6];
	white_queenside_castle_king_attack_mask = king_attack_tables[4] | king_attack_tables[3] | king_attack_tables[2];
	black_kingside_castle_king_attack_mask = king_attack_tables[60] | king_attack_tables[61] | king_attack_tables[62];
	black_queenside_castle_king_attack_mask = king_attack_tables[60] | king_attack_tables[59] | king_attack_tables[58];
}


PieceType MoveGenerator::get_piece_type(const Bitboard* P, unsigned int to_square, unsigned int side)
{
	Bitboard mask = 1ULL << to_square;
	if (P[PAWN * 2 + side] & mask) return PAWN;
	if (P[KNIGHT * 2 + side] & mask) return KNIGHT;
	if (P[BISHOP * 2 + side] & mask) return BISHOP;
	if (P[ROOK * 2 + side] & mask) return ROOK;
	if (P[QUEEN * 2 + side] & mask) return QUEEN;
	if (P[KING * 2 + side] & mask) return KING;
}


void MoveGenerator::generate_pseudo_legal_attacks(uint8_t side_to_move)
{
	std::memset(all_attacks_count+side_to_move, 0, 64);
	int move_idx = 0;
	//bishops moves
	Bitboard bishops = board->P[BISHOP][side_to_move];
	Bitboard rooks = board->P[ROOK][side_to_move];
	Bitboard queens = board->P[QUEEN][side_to_move];
	Bitboard knights = board->P[KNIGHT][side_to_move];
	Bitboard king = board->P[KING][side_to_move];
	Bitboard pawns = board->P[PAWN][side_to_move];
	uint32_t opp = 1 - side_to_move;
	unsigned long from;
	unsigned long to;
	Bitboard relevant_blockers;
	Bitboard relevant_blockers_bishop;
	Bitboard relevant_blockers_rook;
	uint32_t index;
	uint32_t index2;
	Bitboard attacks;
	while (bishops)
	{
		_BitScanForward64(&from, bishops);
		relevant_blockers = board->all_pieces & bishop_relevant_blockers[from];
		index = uint64_t((relevant_blockers * bishop_magic_numbers[from]) >> bishop_relevant_bits_shift[from]);
		attacks = bishop_attack_tables[from][index] & ~board->all_pieces_types[side_to_move];
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			++all_attacks_count[side_to_move][to];
			attacks &= attacks - 1;
		}
		bishops &= bishops - 1;
	}
	//rooks moves
	while (rooks)
	{
		_BitScanForward64(&from, rooks);
		relevant_blockers = board->all_pieces & rook_relevant_blockers[from];
		index = uint64_t((relevant_blockers * rook_magic_numbers[from]) >> rook_relevant_bits_shift[from]);
		attacks = rook_attack_tables[from][index] & ~board->all_pieces_types[side_to_move];
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			attacks &= attacks - 1;
			++all_attacks_count[side_to_move][to];
		}
		rooks &= rooks - 1;
	}
	//queens moves
	while (queens)
	{
		_BitScanForward64(&from, queens);
		relevant_blockers_rook = board->all_pieces & rook_relevant_blockers[from];
		relevant_blockers_bishop = board->all_pieces & bishop_relevant_blockers[from];
		index = uint64_t((relevant_blockers_rook * rook_magic_numbers[from]) >> rook_relevant_bits_shift[from]);
		index2 = uint64_t((relevant_blockers_bishop * bishop_magic_numbers[from]) >> bishop_relevant_bits_shift[from]);
		attacks = (rook_attack_tables[from][index] | bishop_attack_tables[from][index2]) & ~board->all_pieces_types[side_to_move];
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			attacks &= attacks - 1;
			++all_attacks_count[side_to_move][to];
		}
		queens &= queens - 1;
	}
	//knights moves
	while (knights)
	{
		_BitScanForward64(&from, knights);
		attacks = knight_attack_tables[from] & ~board->all_pieces_types[side_to_move];
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			attacks &= attacks - 1;
			++all_attacks_count[side_to_move][to];
		}
		knights &= knights - 1;
	}
	//king moves
	_BitScanForward64(&from, king);
	//if (board == nullptr) abort();
	//if(king_attack_tables == nullptr) abort();
	//volatile uint64_t t1 = board->all_pieces_types[side_to_move];
	//volatile uint64_t t2 = king_attack_tables[0];
	//volatile uint8_t t3 = from;
	//std::cout << from << "  " << (side_to_move ? "1" : "0") << "\n" << std::flush;
	//outFile << from << "  " << (side_to_move ? "1" : "0") << "\n" << std::flush;
	attacks = king_attack_tables[from] & ~board->all_pieces_types[side_to_move];
	while (attacks)
	{    
		_BitScanForward64(&to, attacks);
		attacks &= attacks - 1;
		++all_attacks_count[side_to_move][to];
	}
	//pawn moves
	Bitboard single_push;
	Bitboard double_push;
	Bitboard left_captures;
	Bitboard right_captures;
	if (side_to_move == 0) //white to move
	{
		left_captures = ((pawns & ~0x0101010101010101) << 7) & board->all_pieces_types[1];
		right_captures = ((pawns & ~0x8080808080808080) << 9) & board->all_pieces_types[1];
		
		while (left_captures)
		{
			_BitScanForward64(&to, left_captures);
			++all_attacks_count[side_to_move][to];
			left_captures &= left_captures - 1;
		}
		while (right_captures)
		{
			_BitScanForward64(&to, right_captures);
			++all_attacks_count[side_to_move][to];
			right_captures &= right_captures - 1;
		}
	}
	else//black to move
	{
		
		left_captures = ((pawns & ~0x0101010101010101) >> 9) & board->all_pieces_types[0];
		right_captures = ((pawns & ~0x8080808080808080) >> 7) & board->all_pieces_types[0];
		while (left_captures)
		{
			_BitScanForward64(&to, left_captures);
			++all_attacks_count[side_to_move][to];
			left_captures &= left_captures - 1;
		}
		while (right_captures)
		{
			_BitScanForward64(&to, right_captures);
			++all_attacks_count[side_to_move][to];
			right_captures &= right_captures - 1;
		}

	}
	//en passant captures
	if (board->en_passant_square != 0)
	{
		//constexpr Bitboard FILE_A_NEGATION = ~0x0101010101010101;
		//constexpr Bitboard FILE_H_NEGATION = ~0x8080808080808080;
		Bitboard ep_mask = 1ULL << board->en_passant_square;
		Bitboard pawns_left;
		Bitboard pawns_right;
		if (side_to_move == 0) //white to move
		{
			pawns_left = ((pawns & FILE_A_NEGATION) << 7) & ep_mask;
			pawns_right = ((pawns & FILE_H_NEGATION) << 9) & ep_mask;
			if (pawns_left)
			{
				_BitScanForward64(&to, pawns_left);
				++all_attacks_count[side_to_move][to];
			}
			else if (pawns_right)
			{
				_BitScanForward64(&to, pawns_right);
				++all_attacks_count[side_to_move][to];
			}
		}
		else //black to move
		{
			pawns_left = ((pawns & FILE_A_NEGATION) >> 9) & ep_mask;
			pawns_right = ((pawns & FILE_H_NEGATION) >> 7) & ep_mask;
			if (pawns_left)
			{
				_BitScanForward64(&to, pawns_left);
				++all_attacks_count[side_to_move][to];
			}
			else if (pawns_right)
			{
				_BitScanForward64(&to, pawns_right);
				++all_attacks_count[side_to_move][to];
			}
		}
	}
	//legal_moves_count = move_idx;

}

void MoveGenerator::generate_from_to_masks()
{
	for (int i = 0; i < 4096; ++i)
	{
		from_mask[i] = 1ULL << (i & 0b111111);
		to_mask[i] = 1ULL << (i >> 6);
		from_to_mask[i] = from_mask[i] ^ to_mask[i];
		to_negation_mask[i] = ~to_mask[i];
	}
}



bool inline MoveGenerator::can_castle(uint8_t castle_type)//0 - white kingside, 1 - white queenside, 2 - black kingside, 3 - black queenside
{
	int opp = board->side_to_move ^ 1;
	Bitboard bishop_potential_attacks;
	Bitboard rook_potential_attacks;
	switch (castle_type)
	{
	case 0://white kingside
		//4 - e1 square
		bishop_potential_attacks = bishop_attack_tables[4][(((board->all_pieces & bishop_relevant_blockers[4]) * bishop_magic_numbers[4]) >> bishop_relevant_bits_shift[4])];
		rook_potential_attacks = rook_attack_tables[4][(((board->all_pieces & rook_relevant_blockers[4]) * rook_magic_numbers[4]) >> rook_relevant_bits_shift[4])];

		bishop_potential_attacks |= bishop_attack_tables[5][(((board->all_pieces & bishop_relevant_blockers[5]) * bishop_magic_numbers[5]) >> bishop_relevant_bits_shift[5])];
		rook_potential_attacks |= rook_attack_tables[5][(((board->all_pieces & rook_relevant_blockers[5]) * rook_magic_numbers[5]) >> rook_relevant_bits_shift[5])];

		bishop_potential_attacks |= bishop_attack_tables[6][(((board->all_pieces & bishop_relevant_blockers[6]) * bishop_magic_numbers[6]) >> bishop_relevant_bits_shift[6])];
		rook_potential_attacks |= rook_attack_tables[6][(((board->all_pieces & rook_relevant_blockers[6]) * rook_magic_numbers[6]) >> rook_relevant_bits_shift[6])];

		if (((bishop_potential_attacks | rook_potential_attacks) & board->P[QUEEN][opp]) || (rook_potential_attacks & board->P[ROOK][opp]) || (bishop_potential_attacks & board->P[BISHOP][opp])) return false;
		if (white_kingside_castle_knight_attack_mask & board->P[KNIGHT][opp]) return false;
		if (white_kingside_castle_pawn_attack_mask & board->P[PAWN][opp]) return false;
		if (white_kingside_castle_king_attack_mask & board->P[KING][opp]) return false;
		return true;
	case 1://white queenside
		//4 - e1 square
		bishop_potential_attacks = bishop_attack_tables[4][(((board->all_pieces & bishop_relevant_blockers[4]) * bishop_magic_numbers[4]) >> bishop_relevant_bits_shift[4])];
		rook_potential_attacks = rook_attack_tables[4][(((board->all_pieces & rook_relevant_blockers[4]) * rook_magic_numbers[4]) >> rook_relevant_bits_shift[4])];

		bishop_potential_attacks |= bishop_attack_tables[3][(((board->all_pieces & bishop_relevant_blockers[3]) * bishop_magic_numbers[3]) >> bishop_relevant_bits_shift[3])];
		rook_potential_attacks |= rook_attack_tables[3][(((board->all_pieces & rook_relevant_blockers[3]) * rook_magic_numbers[3]) >> rook_relevant_bits_shift[3])];

		bishop_potential_attacks |= bishop_attack_tables[2][(((board->all_pieces & bishop_relevant_blockers[2]) * bishop_magic_numbers[2]) >> bishop_relevant_bits_shift[2])];
		rook_potential_attacks |= rook_attack_tables[2][(((board->all_pieces & rook_relevant_blockers[2]) * rook_magic_numbers[2]) >> rook_relevant_bits_shift[2])];

		if ((bishop_potential_attacks | rook_potential_attacks) & board->P[QUEEN][opp] || (rook_potential_attacks & board->P[ROOK][opp]) || (bishop_potential_attacks & board->P[BISHOP][opp])) return false;
		if (white_queenside_castle_knight_attack_mask & board->P[KNIGHT][opp]) return false;
		if (white_queenside_castle_pawn_attack_mask & board->P[PAWN][opp]) return false;
		if (white_queenside_castle_king_attack_mask & board->P[KING][opp]) return false;
		return true;
	case 2://black kingside
		//60 - e8 square
		bishop_potential_attacks = bishop_attack_tables[60][(((board->all_pieces & bishop_relevant_blockers[60]) * bishop_magic_numbers[60]) >> bishop_relevant_bits_shift[60])];
		rook_potential_attacks = rook_attack_tables[60][(((board->all_pieces & rook_relevant_blockers[60]) * rook_magic_numbers[60]) >> rook_relevant_bits_shift[60])];

		bishop_potential_attacks |= bishop_attack_tables[61][(((board->all_pieces & bishop_relevant_blockers[61]) * bishop_magic_numbers[61]) >> bishop_relevant_bits_shift[61])];
		rook_potential_attacks |= rook_attack_tables[61][(((board->all_pieces & rook_relevant_blockers[61]) * rook_magic_numbers[61]) >> rook_relevant_bits_shift[61])];

		bishop_potential_attacks |= bishop_attack_tables[62][(((board->all_pieces & bishop_relevant_blockers[62]) * bishop_magic_numbers[62]) >> bishop_relevant_bits_shift[62])];
		rook_potential_attacks |= rook_attack_tables[62][(((board->all_pieces & rook_relevant_blockers[62]) * rook_magic_numbers[62]) >> rook_relevant_bits_shift[62])];

		if ((bishop_potential_attacks | rook_potential_attacks) & board->P[QUEEN][opp] || (rook_potential_attacks & board->P[ROOK][opp]) || (bishop_potential_attacks & board->P[BISHOP][opp])) return false;
		if (black_kingside_castle_knight_attack_mask & board->P[KNIGHT][opp]) return false;
		if (black_kingside_castle_pawn_attack_mask & board->P[PAWN][opp]) return false;
		if (black_kingside_castle_king_attack_mask & board->P[KING][opp]) return false;
		return true;
	case 3://black queenside
		//60 - e8 square
		bishop_potential_attacks = bishop_attack_tables[60][(((board->all_pieces & bishop_relevant_blockers[60]) * bishop_magic_numbers[60]) >> bishop_relevant_bits_shift[60])];
		rook_potential_attacks = rook_attack_tables[60][(((board->all_pieces & rook_relevant_blockers[60]) * rook_magic_numbers[60]) >> rook_relevant_bits_shift[60])];

		bishop_potential_attacks |= bishop_attack_tables[59][(((board->all_pieces & bishop_relevant_blockers[59]) * bishop_magic_numbers[59]) >> bishop_relevant_bits_shift[59])];
		rook_potential_attacks |= rook_attack_tables[59][(((board->all_pieces & rook_relevant_blockers[59]) * rook_magic_numbers[59]) >> rook_relevant_bits_shift[59])];

		bishop_potential_attacks |= bishop_attack_tables[58][(((board->all_pieces & bishop_relevant_blockers[58]) * bishop_magic_numbers[58]) >> bishop_relevant_bits_shift[58])];
		rook_potential_attacks |= rook_attack_tables[58][(((board->all_pieces & rook_relevant_blockers[58]) * rook_magic_numbers[58]) >> rook_relevant_bits_shift[58])];

		if ((bishop_potential_attacks | rook_potential_attacks) & board->P[QUEEN][opp] || (rook_potential_attacks & board->P[ROOK][opp]) || (bishop_potential_attacks & board->P[BISHOP][opp])) return false;
		if (black_queenside_castle_knight_attack_mask & board->P[KNIGHT][opp]) return false;
		if (black_queenside_castle_pawn_attack_mask & board->P[PAWN][opp]) return false;
		if (black_queenside_castle_king_attack_mask & board->P[KING][opp]) return false;
		return true;
	}
}


void MoveGenerator::generate_pseudo_legal_moves_with_category_ordering()
{
	/*
	Generated pseudo legal moves are in the following order:
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
	pseudo_legal_moves_last_idx = -1;


	int side_to_move = board->side_to_move;
	int opp = side_to_move ^ 1;

	Bitboard piece_copy;


	Bitboard single_push;
	Bitboard double_push;

	Bitboard left_captures;
	Bitboard right_captures;

	Bitboard single_push_promotion;
	Bitboard left_captures_promotion;
	Bitboard right_captures_promotion;

	unsigned long to;
	unsigned long from;

	Bitboard attacks;
	Bitboard to_mask;
	Bitboard relevant_blockers;
	uint32_t index;
	uint32_t index2;
	Bitboard relevant_blockers_2;


	piece_copy = board->P[PAWN][side_to_move];

	//PAWNS
	if (side_to_move == 0)//white to move
	{
		single_push = ((piece_copy & RANK_7_NEGATION) << 8) & ~board->all_pieces;//quiet, no promotion
		double_push = ((single_push & RANK_3) << 8) & ~board->all_pieces;

		left_captures = ((piece_copy & FILE_A_NEGATION & RANK_7_NEGATION) << 7) & board->all_pieces_types[1];//no promotion
		right_captures = ((piece_copy & FILE_H_NEGATION & RANK_7_NEGATION) << 9) & board->all_pieces_types[1];//no promotion

		single_push_promotion = ((piece_copy & RANK_7) << 8) & ~board->all_pieces;
		left_captures_promotion = ((piece_copy & FILE_A_NEGATION & RANK_7) << 7) & board->all_pieces_types[1];
		right_captures_promotion = ((piece_copy & FILE_H_NEGATION & RANK_7) << 9) & board->all_pieces_types[1];

		//PAWNS QUIETS
		while (single_push)
		{
			_BitScanForward64(&to, single_push);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to - 8) | (to << 6);
			single_push &= single_push - 1;
		}
		while (double_push)
		{
			_BitScanForward64(&to, double_push);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to - 16) | (to << 6);
			double_push &= double_push - 1;
		}
		pseudo_legal_moves_indexes.quiet_pawn = pseudo_legal_moves_last_idx;
		//PAWNS CAPTURES
		while (left_captures)
		{
			_BitScanForward64(&to, left_captures);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to - 7) | (to << 6);
			left_captures &= left_captures - 1;
		}
		while (right_captures)
		{
			_BitScanForward64(&to, right_captures);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to - 9) | (to << 6);
			right_captures &= right_captures - 1;
		}
		//en passant captures
		if (board->en_passant_square != 0)
		{
			Bitboard en_passant_mask = 1ULL << board->en_passant_square;
			Bitboard pawns_left = (piece_copy & FILE_A_NEGATION) << 7;
			Bitboard pawns_right = (piece_copy & FILE_H_NEGATION) << 9;
			if (pawns_left & en_passant_mask)
			{
				pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (board->en_passant_square - 7) | (board->en_passant_square << 6);
			}
			if (pawns_right & en_passant_mask)
			{
				pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (board->en_passant_square - 9) | (board->en_passant_square << 6);
			}
		}
		pseudo_legal_moves_indexes.pawn_capture = pseudo_legal_moves_last_idx;
		//PAWNS PROMOTIONS
		while (single_push_promotion)
		{
			_BitScanForward64(&to, single_push_promotion);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to - 8) | (to << 6);
			single_push_promotion &= single_push_promotion - 1;
		}
		while (left_captures_promotion)
		{
			_BitScanForward64(&to, left_captures_promotion);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to - 7) | (to << 6);
			left_captures_promotion &= left_captures_promotion - 1;
		}
		while (right_captures_promotion)
		{
			_BitScanForward64(&to, right_captures_promotion);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to - 9) | (to << 6);
			right_captures_promotion &= right_captures_promotion - 1;
		}
		pseudo_legal_moves_indexes.queen_promotion = pseudo_legal_moves_last_idx;
	}
	else//black to move
	{
		single_push = ((piece_copy & RANK_2_NEGATION) >> 8) & ~board->all_pieces;//quiet, no promotion
		double_push = ((single_push & RANK_6) >> 8) & ~board->all_pieces;

		left_captures = ((piece_copy & FILE_A_NEGATION & RANK_2_NEGATION) >> 9) & board->all_pieces_types[0];//no promotion
		right_captures = ((piece_copy & FILE_H_NEGATION & RANK_2_NEGATION) >> 7) & board->all_pieces_types[0];//no promotion

		single_push_promotion = ((piece_copy & RANK_2) >> 8) & ~board->all_pieces;
		left_captures_promotion = ((piece_copy & FILE_A_NEGATION & RANK_2) >> 9) & board->all_pieces_types[0];
		right_captures_promotion = ((piece_copy & FILE_H_NEGATION & RANK_2) >> 7) & board->all_pieces_types[0];

		//PAWNS QUIETS
		while (single_push)
		{
			_BitScanForward64(&to, single_push);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to + 8) | (to << 6);
			single_push &= single_push - 1;
		}
		while (double_push)
		{
			_BitScanForward64(&to, double_push);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to + 16) | (to << 6);
			double_push &= double_push - 1;
		}
		pseudo_legal_moves_indexes.quiet_pawn = pseudo_legal_moves_last_idx;
		//PAWNS CAPTURES
		while (left_captures)
		{
			_BitScanForward64(&to, left_captures);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to + 9) | (to << 6);
			left_captures &= left_captures - 1;
		}
		while (right_captures)
		{
			_BitScanForward64(&to, right_captures);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to + 7) | (to << 6);
			right_captures &= right_captures - 1;
		}
		//en passant captures
		if (board->en_passant_square != 0)
		{
			Bitboard en_passant_mask = 1ULL << board->en_passant_square;
			Bitboard pawns_left = (piece_copy & FILE_A_NEGATION) >> 9;
			Bitboard pawns_right = (piece_copy & FILE_H_NEGATION) >> 7;
			if (pawns_left & en_passant_mask)
			{
				pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (board->en_passant_square + 9) | (board->en_passant_square << 6);
			}
			if (pawns_right & en_passant_mask)
			{
				pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (board->en_passant_square + 7) | (board->en_passant_square << 6);
			}
		}
		pseudo_legal_moves_indexes.pawn_capture = pseudo_legal_moves_last_idx;
		//PAWNS PROMOTIONS
		while (single_push_promotion)
		{
			_BitScanForward64(&to, single_push_promotion);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to + 8) | (to << 6);
			single_push_promotion &= single_push_promotion - 1;
		}
		while (left_captures_promotion)
		{
			_BitScanForward64(&to, left_captures_promotion);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to + 9) | (to << 6);
			left_captures_promotion &= left_captures_promotion - 1;
		}
		while (right_captures_promotion)
		{
			_BitScanForward64(&to, right_captures_promotion);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = (to + 7) | (to << 6);
			right_captures_promotion &= right_captures_promotion - 1;
		}
		pseudo_legal_moves_indexes.queen_promotion = pseudo_legal_moves_last_idx;
	}

	//KNIGHT CAPTURES
	piece_copy = board->P[KNIGHT][side_to_move];
	while (piece_copy)
	{
		_BitScanForward64(&from, piece_copy);
		attacks = knight_attack_tables[from] & ~board->all_pieces_types[side_to_move] & board->all_pieces_types[opp];
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = from | (to << 6);
			attacks &= attacks - 1;		
		}
		piece_copy &= piece_copy - 1;
	}
	pseudo_legal_moves_indexes.knight_capture = pseudo_legal_moves_last_idx;
	//KNIGHT QUIETS
	piece_copy = board->P[KNIGHT][side_to_move];
	while (piece_copy)
	{
		_BitScanForward64(&from, piece_copy);
		attacks = knight_attack_tables[from] & ~board->all_pieces;
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = from | (to << 6);
			attacks &= attacks - 1;
		}
		piece_copy &= piece_copy - 1;
	}
	pseudo_legal_moves_indexes.quiet_knight = pseudo_legal_moves_last_idx;

	//BISHOP CAPTURES
	piece_copy = board->P[BISHOP][side_to_move];
	while (piece_copy)
	{
		_BitScanForward64(&from, piece_copy);
		relevant_blockers = board->all_pieces & bishop_relevant_blockers[from];
		index = uint64_t((relevant_blockers * bishop_magic_numbers[from]) >> bishop_relevant_bits_shift[from]);
		attacks = bishop_attack_tables[from][index] & ~board->all_pieces_types[side_to_move] & board->all_pieces_types[opp];
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = from | (to << 6);
			attacks &= attacks - 1;
		}
		piece_copy &= piece_copy - 1;
	}
	pseudo_legal_moves_indexes.bishop_capture = pseudo_legal_moves_last_idx;
	//BISHOP QUIETS
	piece_copy = board->P[BISHOP][side_to_move];
	while (piece_copy)
	{
		_BitScanForward64(&from, piece_copy);
		relevant_blockers = board->all_pieces & bishop_relevant_blockers[from];
		index = uint64_t((relevant_blockers * bishop_magic_numbers[from]) >> bishop_relevant_bits_shift[from]);
		attacks = bishop_attack_tables[from][index] & ~board->all_pieces;
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = from | (to << 6);
			attacks &= attacks - 1;
		}
		piece_copy &= piece_copy - 1;
	}
	pseudo_legal_moves_indexes.quiet_bishop = pseudo_legal_moves_last_idx;

	//ROOK CAPTURES
	piece_copy = board->P[ROOK][side_to_move];
	while (piece_copy)
	{
		_BitScanForward64(&from, piece_copy);
		relevant_blockers = board->all_pieces & rook_relevant_blockers[from];
		index = uint64_t((relevant_blockers * rook_magic_numbers[from]) >> rook_relevant_bits_shift[from]);
		attacks = rook_attack_tables[from][index] & ~board->all_pieces_types[side_to_move] & board->all_pieces_types[opp];
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = from | (to << 6);
			attacks &= attacks - 1;
		}
		piece_copy &= piece_copy - 1;
	}
	pseudo_legal_moves_indexes.rook_capture = pseudo_legal_moves_last_idx;
	//ROOK QUIETS
	piece_copy = board->P[ROOK][side_to_move];
	while (piece_copy)
	{
		_BitScanForward64(&from, piece_copy);
		relevant_blockers = board->all_pieces & rook_relevant_blockers[from];
		index = uint64_t((relevant_blockers * rook_magic_numbers[from]) >> rook_relevant_bits_shift[from]);
		attacks = rook_attack_tables[from][index] & ~board->all_pieces;
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = from | (to << 6);
			attacks &= attacks - 1;
		}
		piece_copy &= piece_copy - 1;
	}
	pseudo_legal_moves_indexes.quiet_rook = pseudo_legal_moves_last_idx;


	//QUEEN CAPTURES
	piece_copy = board->P[QUEEN][side_to_move];
	while (piece_copy)
	{
		_BitScanForward64(&from, piece_copy);

		relevant_blockers = board->all_pieces & rook_relevant_blockers[from];
		relevant_blockers_2 = board->all_pieces & bishop_relevant_blockers[from];
		index = uint64_t((relevant_blockers * rook_magic_numbers[from]) >> rook_relevant_bits_shift[from]);
		index2 = uint64_t((relevant_blockers_2 * bishop_magic_numbers[from]) >> bishop_relevant_bits_shift[from]);
		attacks = (rook_attack_tables[from][index] | bishop_attack_tables[from][index2]) & ~board->all_pieces_types[side_to_move] & board->all_pieces_types[opp];
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = from | (to << 6);
			attacks &= attacks - 1;
		}
		piece_copy &= piece_copy - 1;
	}
	pseudo_legal_moves_indexes.queen_capture = pseudo_legal_moves_last_idx;
	//QUEEN QUIETS
	piece_copy = board->P[QUEEN][side_to_move];
	while (piece_copy)
	{
		_BitScanForward64(&from, piece_copy);

		relevant_blockers = board->all_pieces & rook_relevant_blockers[from];
		relevant_blockers_2 = board->all_pieces & bishop_relevant_blockers[from];
		index = uint64_t((relevant_blockers * rook_magic_numbers[from]) >> rook_relevant_bits_shift[from]);
		index2 = uint64_t((relevant_blockers_2 * bishop_magic_numbers[from]) >> bishop_relevant_bits_shift[from]);
		attacks = (rook_attack_tables[from][index] | bishop_attack_tables[from][index2]) & ~board->all_pieces;
		while (attacks)
		{
			_BitScanForward64(&to, attacks);
			pseudo_legal_moves[++pseudo_legal_moves_last_idx] = from | (to << 6);
			attacks &= attacks - 1;
		}
		piece_copy &= piece_copy - 1;
	}
	pseudo_legal_moves_indexes.quiet_queen = pseudo_legal_moves_last_idx;

	//KING CAPTURES
	_BitScanForward64(&from, board->P[KING][side_to_move]);
	attacks = king_attack_tables[from] & ~board->all_pieces_types[side_to_move] & board->all_pieces_types[opp];
	while (attacks)
	{
		_BitScanForward64(&to, attacks);
		pseudo_legal_moves[++pseudo_legal_moves_last_idx] = from | (to << 6);
		attacks &= attacks - 1;
	}
	pseudo_legal_moves_indexes.king_capture = pseudo_legal_moves_last_idx;
	//KING QUIETS
	attacks = king_attack_tables[from] & ~board->all_pieces;
	while (attacks)
	{
		_BitScanForward64(&to, attacks);
		pseudo_legal_moves[++pseudo_legal_moves_last_idx] = from | (to << 6);
		attacks &= attacks - 1;
	}
	pseudo_legal_moves_indexes.quiet_king = pseudo_legal_moves_last_idx;

	//CASTLES
	if (side_to_move == 0)//white to move
	{
		if (board->castling_rights & 0b0001)//white king side
		{
			if ((board->all_pieces & WHITE_KINGSIDE_CASTLE_MASK) == 0 && can_castle(0))
			{
				pseudo_legal_moves[++pseudo_legal_moves_last_idx] = WHITE_KINGSIDE_CASTLE_FROM_TO_MASK;
			}
		}
		if (board->castling_rights & 0b0010)//white queen side
		{
			if ((board->all_pieces & WHITE_QUEENSIDE_CASTLE_MASK) == 0 && can_castle(1))
			{
				pseudo_legal_moves[++pseudo_legal_moves_last_idx] = WHITE_QUEENSIDE_CASTLE_FROM_TO_MASK;
			}
		}
	}
	else//black to move
	{
		if (board->castling_rights & 0b0100)//black king side
		{
			if ((board->all_pieces & BLACK_KINGSIDE_CASTLE_MASK) == 0 && can_castle(2))
			{
				pseudo_legal_moves[++pseudo_legal_moves_last_idx] = BLACK_KINGSIDE_CASTLE_FROM_TO_MASK;
			}
		}
		if (board->castling_rights & 0b1000)//black queen side
		{
			if ((board->all_pieces & BLACK_QUEENSIDE_CASTLE_MASK) == 0 && can_castle(3))
			{
				pseudo_legal_moves[++pseudo_legal_moves_last_idx] = BLACK_QUEENSIDE_CASTLE_FROM_TO_MASK;
			}
		}
	}
	pseudo_legal_moves_indexes.castle = pseudo_legal_moves_last_idx;

}


char* get_square_name(int square)
{
	static char name[3];
	const char files[] = "abcdefgh";
	const char ranks[] = "12345678";
	name[0] = files[square % 8];
	name[1] = ranks[square / 8];
	name[2] = '\0';
	return name;
}

std::string get_piece_name(PieceType piece)
{
	switch (piece)
	{
	case PAWN:
		return "Pawn";
	case KNIGHT:
		return "Knight";
	case BISHOP:
		return "Bishop";
	case ROOK:
		return "Rook";
	case QUEEN:
		return "Queen";
	case KING:
		return "King";
	case EMPTY:
		return "EMPTY";
	case NONE:
		return "None";
	default:
		return "?";
	}
}






constexpr uint16_t index_max_value = 0xFF;

void MoveGenerator::filter_pseudo_legal_moves()
{
	/*
	Pseudo legal moves are stored in a following order:
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


	legal_moves_last_idx = -1;

	unsigned long king_square;
	int side_to_move = board->side_to_move;
	int opp = side_to_move ^ 1;
	_BitScanForward64(&king_square, board->P[KING][side_to_move]);
	int to;

	int index = uint64_t(((board->all_pieces & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]);
 	Bitboard bishop_potential_attacks = bishop_attack_tables[king_square][index];
	Bitboard blockers_bishop = bishop_blockers[king_square][index];

	index = uint64_t(((board->all_pieces & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]);
	Bitboard rook_potential_attacks = rook_attack_tables[king_square][index];
	Bitboard blockers_rook = rook_blockers[king_square][index];

	Bitboard queen_potential_attacks = bishop_potential_attacks | rook_potential_attacks;


	//check if king is in check before any moves
	knight_check = knight_attack_tables[king_square] & board->P[KNIGHT][opp];
	pawn_check = pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp];
	bishop_check = bishop_potential_attacks & board->P[BISHOP][opp];
	rook_check = rook_potential_attacks & board->P[ROOK][opp];
	queen_check = queen_potential_attacks & board->P[QUEEN][opp];
	in_check = knight_check || pawn_check || bishop_check || rook_check || queen_check;
	int i = 0;
	if (in_check)
	{//king in check

		if (!(pawn_check || knight_check))
		{
			//PAWN QUIET
			for (; i < ((pseudo_legal_moves_indexes.quiet_pawn + 1) &index_max_value); ++i)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.quiet_pawn = legal_moves_last_idx;
			//PAWN CAPTURES
			for (; i < ((pseudo_legal_moves_indexes.pawn_capture + 1) &index_max_value); ++i)
			{
				if (board->en_passant_square != 0 && pseudo_legal_moves[i] >> 6 == board->en_passant_square)
				{
					//en passant special case
					Bitboard ep_captured_pawn_mask;
					if (side_to_move == 0)//white to move
						ep_captured_pawn_mask = to_mask[pseudo_legal_moves[i]] >> 8;
					else
						ep_captured_pawn_mask = to_mask[pseudo_legal_moves[i]] << 8;
					//remove the captured pawn from the board temporarily and check for checks
					bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]] ^ ep_captured_pawn_mask) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
					if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
					rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]] ^ ep_captured_pawn_mask) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
					if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
					legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
					continue;
				}

				//check for sliding pieces checks after the move (the piece was pined)
				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				//check the pawn on the end so the move does not get accepted even thought the piece was pined
				if (pawn_check)//cannot be a discovered check, it's the only check, if it is pawn check and only 1 panw is checking
				{
					if (to_mask[pseudo_legal_moves[i]] & (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]))//we check if the move captures the checking pawn
					{
						legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
						continue;
					}
					else
					{
						continue;
					}
				}
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.pawn_capture = legal_moves_last_idx;
			//PROMOTIONS TO QUEEN
			for (; i < ((pseudo_legal_moves_indexes.queen_promotion + 1) & index_max_value); ++i)//promotion may or may not be captures
			{
				if (to_mask[pseudo_legal_moves[i]] & board->all_pieces_types[opp])//capture
				{
					
					bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
					if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
					rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
					if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
					legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
				}
				else//promotion without capture
				{
					bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
					if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
					rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
					if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
					legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
				}
			}
			legal_moves_indexes.queen_promotion = legal_moves_last_idx;
			//KNIGHT CAPTURES
			for (; i < ((pseudo_legal_moves_indexes.knight_capture + 1) &index_max_value); ++i)
			{

				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.knight_capture = legal_moves_last_idx;
			//KNIGHT QUIET
			for (; i < ((pseudo_legal_moves_indexes.quiet_knight + 1) &index_max_value); ++i)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.quiet_knight = legal_moves_last_idx;
			//BISHOP CAPTURES
			for (; i < ((pseudo_legal_moves_indexes.bishop_capture + 1) &index_max_value); ++i)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.bishop_capture = legal_moves_last_idx;
			//BISHOP QUIET
			for (; i < ((pseudo_legal_moves_indexes.quiet_bishop + 1) &index_max_value); ++i)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.quiet_bishop = legal_moves_last_idx;
			//ROOK CAPTURES
			for (; i < ((pseudo_legal_moves_indexes.rook_capture + 1) &index_max_value); ++i)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.rook_capture = legal_moves_last_idx;
			//ROOK QUIET
			for (; i < ((pseudo_legal_moves_indexes.quiet_rook + 1) &index_max_value); ++i)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.quiet_rook = legal_moves_last_idx;
			//QUEEN CAPTURES
			for (; i < ((pseudo_legal_moves_indexes.queen_capture + 1) &index_max_value); ++i)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.queen_capture = legal_moves_last_idx;
			//QUEEN QUIET
			for (; i < ((pseudo_legal_moves_indexes.quiet_queen + 1) &index_max_value); ++i)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.quiet_queen = legal_moves_last_idx;
			//KING CAPTURES
			for (; i < ((pseudo_legal_moves_indexes.king_capture + 1) &index_max_value); ++i)
			{
				//check if check after the move from the ground (we don't have any relevan information for optimalization)
				king_square = pseudo_legal_moves[i] >> 6;
				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] & to_negation_mask[pseudo_legal_moves[i]]) || rook_potential_attacks & (board->P[ROOK][opp] & to_negation_mask[pseudo_legal_moves[i]]) || (bishop_potential_attacks | rook_potential_attacks) & (board->P[QUEEN][opp] & to_negation_mask[pseudo_legal_moves[i]])) continue;
				if (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]) continue;//we check pawn attack talbe from our side to check king perspective
				if (knight_attack_tables[king_square] & board->P[KNIGHT][opp]) continue;
				//check for opponen's king
				if (king_attack_tables[king_square] & board->P[KING][opp]) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.king_capture = legal_moves_last_idx;
			//KING QUIET
			for (; i < ((pseudo_legal_moves_indexes.quiet_king + 1) &index_max_value); ++i)
			{
				king_square = pseudo_legal_moves[i] >> 6;
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
				if (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]) continue;//we check pawn attack talbe from our side to check king perspective
				if (knight_attack_tables[king_square] & board->P[KNIGHT][opp]) continue;
				//check for opponen's king
				if (king_attack_tables[king_square] & board->P[KING][opp]) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.quiet_king = legal_moves_last_idx;
			//CASTLING - castling is illegal in check
			legal_moves_indexes.castle = legal_moves_last_idx;
		}
		else
		{
			//PAWN QUIET
			legal_moves_indexes.quiet_pawn = legal_moves_last_idx;
			//PAWN CAPTURES
			for (i = (pseudo_legal_moves_indexes.quiet_pawn+1) & index_max_value; i < ((pseudo_legal_moves_indexes.pawn_capture + 1) &index_max_value); ++i)
			{
				if (knight_check)
					if (knight_attack_tables[king_square] & (board->P[KNIGHT][opp] & to_negation_mask[pseudo_legal_moves[i]])) continue;
				if (board->en_passant_square != 0 && pseudo_legal_moves[i] >> 6 == board->en_passant_square)
				{
					//IMPORTANT 1: When the pawn double moves and the king is in check after the move, unless the king is check dirrectly by the pawn, the en passant move cannot block the check, the is simply no such possition, based on my rather brief analysis
					//IMPORTANT 2: When the king is in check by the pawn after a double move, it is a single check by a pawn, so the en passant capture is always legal as long as the pawn is not pinned
					//en passant special case
					Bitboard ep_captured_pawn_mask;
					if (side_to_move == 0)//white to move
						ep_captured_pawn_mask = to_mask[pseudo_legal_moves[i]] >> 8;
					else
						ep_captured_pawn_mask = to_mask[pseudo_legal_moves[i]] << 8;
					if (pawn_check)//cannot be a discovered check, it's the only check, if it is pawn check and only 1 pawn is checking
					{
						//we only need to check if the pawn is not pinned
						//since the king is in check by the pawn which double moved and an en passant capture is possible the pawn which is taking making an en passant capture can only be pinned if it is on the same file as the king,specificly in front of it
						if (((pseudo_legal_moves[i]&0b111111)%8)==(king_square%8))
						{
							if (rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])] & (board->P[ROOK][opp] | board->P[QUEEN][opp]))
							{
								continue;
							}
							legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
							continue;
						}
						legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
						continue;
					}
					//remove the captured pawn from the board temporarily and check for checks
					bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]] ^ ep_captured_pawn_mask) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
					if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
					rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]] ^ ep_captured_pawn_mask) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
					if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
					legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
					continue;
				}

				//check for sliding pieces checks after the move (the piece was pined)
				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				//check the pawn on the end so the move does not get accepted even thought the piece was pined
				if (pawn_check)//cannot be a discovered check, it's the only check, if it is pawn check and only 1 panw is checking
				{
					if (to_mask[pseudo_legal_moves[i]] & (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]))//we check if the move captures the checking pawn
					{
						legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
						continue;
					}
					else
					{
						continue;
					}
				}
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.pawn_capture = legal_moves_last_idx;
			//PROMOTIONS TO QUEEN
			for (; i < ((pseudo_legal_moves_indexes.queen_promotion + 1) & index_max_value); ++i)//promotion may or may not be captures
			{
				if (to_mask[pseudo_legal_moves[i]] & board->all_pieces_types[opp])//capture
				{
					if (knight_check)
						if (knight_attack_tables[king_square] & (board->P[KNIGHT][opp] & to_negation_mask[pseudo_legal_moves[i]])) continue;
					bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
					if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
					rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
					if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
					if (pawn_check)//cannot be a discovered check, it's the only check, if it is pawn check and only 1 panw is checking
					{
						if (to_mask[pseudo_legal_moves[i]] & (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]))//we check if the move captures the checking pawn
						{
							legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
							continue;
						}
						else
						{
							continue;
						}
					}
					legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
				}
				else//promotion without capture
				{
					if (knight_check || pawn_check)//if pawn or knight is chcking we skip because non capture move cannot remove their check
						continue;
					bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
					if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
					rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
					if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
					legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
				}
			}
			legal_moves_indexes.queen_promotion = legal_moves_last_idx;
			//KNIGHT CAPTURES
			for (; i < ((pseudo_legal_moves_indexes.knight_capture + 1) & index_max_value); ++i)
			{
				if (knight_check)
					if (knight_attack_tables[king_square] & (board->P[KNIGHT][opp] & to_negation_mask[pseudo_legal_moves[i]])) continue;

				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;

				if (pawn_check)//cannot be a discovered check, it's the only check, if it is pawn check and only 1 panw is checking
				{
					if (to_mask[pseudo_legal_moves[i]] & (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]))//we check if the move captures the checking pawn
					{
						legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
						continue;
					}
					else
					{
						continue;
					}
				}
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.knight_capture = legal_moves_last_idx;
			//KNIGHT QUIET
			legal_moves_indexes.quiet_knight = legal_moves_last_idx;
			//BISHOP CAPTURES
			for (i = (pseudo_legal_moves_indexes.quiet_knight +1) &index_max_value; i < ((pseudo_legal_moves_indexes.bishop_capture + 1) & index_max_value); ++i)
			{
				if (knight_check)
					if (knight_attack_tables[king_square] & (board->P[KNIGHT][opp] & to_negation_mask[pseudo_legal_moves[i]])) continue;
				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				if (pawn_check)//cannot be a discovered check, it's the only check, if it is pawn check and only 1 panw is checking
				{
					if (to_mask[pseudo_legal_moves[i]] & (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]))//we check if the move captures the checking pawn
						legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
					continue;
				}
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.bishop_capture = legal_moves_last_idx;
			//BISHOP QUIET
			legal_moves_indexes.quiet_bishop = legal_moves_last_idx;
			//ROOK CAPTURES
			for (i = (pseudo_legal_moves_indexes.quiet_bishop+1) &index_max_value; i < ((pseudo_legal_moves_indexes.rook_capture + 1) & index_max_value); ++i)
			{
				if (knight_check)
					if (knight_attack_tables[king_square] & (board->P[KNIGHT][opp] & to_negation_mask[pseudo_legal_moves[i]])) continue;
				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				if (pawn_check)//cannot be a discovered check, it's the only check, if it is pawn check and only 1 pawn is checking
				{
					if (to_mask[pseudo_legal_moves[i]] & (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]))//we check if the move captures the checking pawn
						legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
					continue;
				}
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.rook_capture = legal_moves_last_idx;
			//ROOK QUIET
			legal_moves_indexes.quiet_rook = legal_moves_last_idx;
			//QUEEN CAPTURES
			for (i = (pseudo_legal_moves_indexes.quiet_rook+1) &index_max_value; i < ((pseudo_legal_moves_indexes.queen_capture + 1) & index_max_value); ++i)
			{
				if (knight_check)
					if (knight_attack_tables[king_square] & (board->P[KNIGHT][opp] & to_negation_mask[pseudo_legal_moves[i]])) continue;
				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				if (pawn_check)//cannot be a discovered check, it's the only check, if it is pawn check and only 1 panw is checking
				{
					if (to_mask[pseudo_legal_moves[i]] & (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]))//we check if the move captures the checking pawn
					{
						legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
						continue;
					}
					else
					{
						continue;
					}
				}
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.queen_capture = legal_moves_last_idx;
			//QUEEN QUIET
			legal_moves_indexes.quiet_queen = legal_moves_last_idx;
			//KING CAPTURES
			for (i = (pseudo_legal_moves_indexes.quiet_queen+1) &index_max_value; i < ((pseudo_legal_moves_indexes.king_capture + 1) & index_max_value); ++i)
			{
				//check if check after the move from the ground (we don't have any relevant information for optimalization)
				king_square = pseudo_legal_moves[i] >> 6;
				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & ((board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & ((board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]])) continue;
				if (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]) continue;//we check pawn attack talbe from our side to check king perspective
				if (knight_attack_tables[king_square] & board->P[KNIGHT][opp]) continue;
				//check for opponen's king
				if (king_attack_tables[king_square] & board->P[KING][opp]) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.king_capture = legal_moves_last_idx;
			//KING QUIET
			for (; i < ((pseudo_legal_moves_indexes.quiet_king + 1) & index_max_value); ++i)
			{
				king_square = pseudo_legal_moves[i] >> 6;
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
				//check for knights and pawns
				if (knight_attack_tables[king_square] & board->P[KNIGHT][opp]) continue;
				if (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]) continue;//side to move because we are checking attacks from king perspective (what squars can be attack from king square)
				//check for oppenens king
				if (king_attack_tables[king_square] & board->P[KING][opp]) continue;
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
			}
			legal_moves_indexes.quiet_king = legal_moves_last_idx;
			//CASTLING - castling is illegal in check
			legal_moves_indexes.castle = legal_moves_last_idx;
		}
	}
	else//king not in check
	{
		//PAWN QUIET
		for (; i < ((pseudo_legal_moves_indexes.quiet_pawn + 1) & index_max_value); ++i)
		{
			if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
			}
			else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
			{
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
			}
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.quiet_pawn = legal_moves_last_idx;
		//PAWN CAPTURES
		for (; i < ((pseudo_legal_moves_indexes.pawn_capture + 1) & index_max_value); ++i)
		{
			if (board->en_passant_square != 0 && pseudo_legal_moves[i] >> 6 == board->en_passant_square)
			{
				//en passant special case
				Bitboard ep_captured_pawn_mask;
				if (side_to_move == 0)//white to move
					ep_captured_pawn_mask = to_mask[pseudo_legal_moves[i]] >> 8;
				else
					ep_captured_pawn_mask = to_mask[pseudo_legal_moves[i]] << 8;

				if ((from_mask[pseudo_legal_moves[i]] | ep_captured_pawn_mask) & blockers_bishop)
				{
					//check if king is attacked by enemy bishop/queen on bishop diagnals after move
					bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]] ^ ep_captured_pawn_mask) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
					if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
				}
				else if ((from_mask[pseudo_legal_moves[i]] | ep_captured_pawn_mask) & blockers_rook)
				{
					//check if king is attacked by enemy rook/queen on rook lines after move
					rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]] ^ ep_captured_pawn_mask) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
					if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
				}
				legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
				continue;
			}
			if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
			{
				//check if king is attacked by enemy bishop/queen on bishop diagnals after move
				bishop_potential_attacks = bishop_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square])];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
			}
			else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
			{
				//check if king is attacked by enemy rook/queen on rook lines after move
				rook_potential_attacks = rook_attack_tables[king_square][((((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square])];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
			}

			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.pawn_capture = legal_moves_last_idx;
		//PROMOTIONS TO QUEEN
		for (; i < ((pseudo_legal_moves_indexes.queen_promotion + 1) & index_max_value); ++i)
		{
			if (to_mask[pseudo_legal_moves[i]] & board->all_pieces_types[opp])//capture
			{
				if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
				{
					bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
					if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
				}
				else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
				{
					rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
					if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
				}
			}
			else//quiet move
			{
				if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
				{
					bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
					if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
				}
				else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
				{
					rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
					if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
				}
			}
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.queen_promotion = legal_moves_last_idx;
		//pseudo legal moves already check if king is in check after the castle so the castle moevs can be just copied
		//KNIGHT CAPTURES
		for (; i < ((pseudo_legal_moves_indexes.knight_capture + 1) & index_max_value); ++i)
		{
			if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
			}
			else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
			{
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
			}
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.knight_capture = legal_moves_last_idx;
		//KNIGHT QUIET
		for (; i < ((pseudo_legal_moves_indexes.quiet_knight + 1) & index_max_value); ++i)
		{
			if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
			}
			else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
			{
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
			}
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.quiet_knight = legal_moves_last_idx;
		//BISHOP CAPTURES
		for (; i < ((pseudo_legal_moves_indexes.bishop_capture + 1) & index_max_value); ++i)
		{
			if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
			}
			else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
			{
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
			}
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.bishop_capture = legal_moves_last_idx;
		//BISHOP QUIET
		for (; i < ((pseudo_legal_moves_indexes.quiet_bishop + 1) & index_max_value); ++i)
		{
			if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
			}
			else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
			{
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
			}
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.quiet_bishop = legal_moves_last_idx;
		//ROOK CAPTURES
		for (; i < ((pseudo_legal_moves_indexes.rook_capture + 1) & index_max_value); ++i)
		{
			if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
			}
			else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
			{
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
			}
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.rook_capture = legal_moves_last_idx;
		//ROOK QUIET
		for (; i < ((pseudo_legal_moves_indexes.quiet_rook + 1) & index_max_value); ++i)
		{
			if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
			}
			else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
			{
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
			}
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.quiet_rook = legal_moves_last_idx;
		//QUEEN CAPTURES
		for (; i < ((pseudo_legal_moves_indexes.queen_capture + 1) & index_max_value); ++i)
		{
			if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
			}
			else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
			{
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp]) & to_negation_mask[pseudo_legal_moves[i]]) continue;
			}
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.queen_capture = legal_moves_last_idx;
		//QUEEN QUIET
		for (; i < ((pseudo_legal_moves_indexes.quiet_queen + 1) & index_max_value); ++i)
		{
			if (from_mask[pseudo_legal_moves[i]] & blockers_bishop)
			{
				bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
				if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
			}
			else if (from_mask[pseudo_legal_moves[i]] & blockers_rook)
			{
				rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
				if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
			}
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.quiet_queen = legal_moves_last_idx;
		//KING CAPTURES
		for (; i < ((pseudo_legal_moves_indexes.king_capture + 1) & index_max_value); ++i)
		{
			king_square = pseudo_legal_moves[i] >> 6;//& 0b111111;
			//check if king is in check after move
			bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
			if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;//not need for  & to_negation_mask[capture_with_king_pseudo_legal_moves[i]] because if the king taken the potential attaacker the attacks do not include the square th piece is on (king being on the same square as queen will not be check)
			rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
			if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;//the same as above
			//check for knights and pawns
			if (knight_attack_tables[king_square] & board->P[KNIGHT][opp]) continue;
			if (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]) continue;//side to move because we are checking attacks from king perspective (what squars can be attack from king square)
			//check for oppenens king
			if (king_attack_tables[king_square] & board->P[KING][opp]) continue;
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.king_capture = legal_moves_last_idx;
		//KING QUIET
		for (; i < ((pseudo_legal_moves_indexes.quiet_king + 1) & index_max_value); ++i)
		{
			king_square = pseudo_legal_moves[i] >> 6;//& 0b111111;
			//check if king is in check after move
			bishop_potential_attacks = bishop_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & bishop_relevant_blockers[king_square]) * bishop_magic_numbers[king_square]) >> bishop_relevant_bits_shift[king_square]];
			if (bishop_potential_attacks & (board->P[BISHOP][opp] | board->P[QUEEN][opp])) continue;
			rook_potential_attacks = rook_attack_tables[king_square][(((board->all_pieces ^ from_to_mask[pseudo_legal_moves[i]]) & rook_relevant_blockers[king_square]) * rook_magic_numbers[king_square]) >> rook_relevant_bits_shift[king_square]];
			if (rook_potential_attacks & (board->P[ROOK][opp] | board->P[QUEEN][opp])) continue;
			//check for knights and pawns
			if (knight_attack_tables[king_square] & board->P[KNIGHT][opp]) continue;
			if (pawn_attack_tables[side_to_move][king_square] & board->P[PAWN][opp]) continue;//side to move because we are checking attacks from king perspective (what squars can be attack from king square)
			//check for oppenens king
			if (king_attack_tables[king_square] & board->P[KING][opp]) continue;
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.quiet_king = legal_moves_last_idx;
		//CASTLING
		for (; i < ((pseudo_legal_moves_indexes.castle + 1) & index_max_value); ++i)
		{//pseudo legal moves generator already check if king is in check after the castle so the castle moves can be just copied
			legal_moves[++legal_moves_last_idx] = pseudo_legal_moves[i];
		}
		legal_moves_indexes.castle = legal_moves_last_idx;
	}
	//if (legal_moves_last_idx >= MoveGenerator::max_legal_moves_count) abort();

}


std::vector<Move> MoveGenerator::get_legal_moves()
{
	/*
	Generated legal moves are in the following order :
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
	int promotions_count = ((legal_moves_indexes.queen_promotion + 1)&index_max_value) - ((legal_moves_indexes.pawn_capture + 1) & index_max_value);
	legal_moves_vector.resize(legal_moves_indexes.castle + 1 + promotions_count * 3);
	int i = 0;
	while (i<((legal_moves_indexes.quiet_pawn + 1) & index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = QUIET_PAWN;
	}
	while (i< ((legal_moves_indexes.pawn_capture + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = CAPTURE_WITH_PAWN;
	}
	while (i< ((legal_moves_indexes.queen_promotion + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = QUEEN_PROMOTION;
	}
	while (i< ((legal_moves_indexes.knight_capture + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = CAPTURE_WITH_KNIGHT;
	}
	while (i< ((legal_moves_indexes.quiet_knight + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = QUIET_KNIGHT;
	}
	while (i< ((legal_moves_indexes.bishop_capture + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = CAPTURE_WITH_BISHOP;
	}
	while (i< ((legal_moves_indexes.quiet_bishop + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = QUIET_BISHOP;
	}
	while (i< ((legal_moves_indexes.rook_capture + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = CAPTURE_WITH_ROOK;
	}
	while (i< ((legal_moves_indexes.quiet_rook + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = QUIET_ROOK;
	}
	while (i< ((legal_moves_indexes.queen_capture + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = CAPTURE_WITH_QUEEN;
	}
	while (i< ((legal_moves_indexes.quiet_queen + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = QUIET_QUEEN;
	}
	while (i< ((legal_moves_indexes.king_capture + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = CAPTURE_WITH_KING;
	}
	while (i< ((legal_moves_indexes.quiet_king + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = QUIET_KING;
	}
	while (i< ((legal_moves_indexes.castle + 1) &index_max_value))
	{
		legal_moves_vector[i].move = legal_moves[i];
		legal_moves_vector[i++].move_type = CASTLE;
	}
	//add knight, rook, bishop promotions
	for (int j = 1; j <= promotions_count; ++j)
	{
		legal_moves_vector[i].move = legal_moves[((legal_moves_indexes.pawn_capture + 1) & index_max_value) + j-1];
		legal_moves_vector[i++].move_type = KNIGHT_PROMOTION;
		legal_moves_vector[i].move = legal_moves[((legal_moves_indexes.pawn_capture + 1) & index_max_value) + j-1];
		legal_moves_vector[i++].move_type = ROOK_PROMOTION;
		legal_moves_vector[i].move = legal_moves[((legal_moves_indexes.pawn_capture + 1) & index_max_value) + j-1];
		legal_moves_vector[i++].move_type = BISHOP_PROMOTION;
		if (((legal_moves_indexes.pawn_capture + 1) & index_max_value) + j - 1 > 219)
			std::abort();
	}
	if (i > 219)
		std::abort();
	return legal_moves_vector;
}

std::vector<Move> MoveGenerator::get_pseudo_legal_moves()
{
	/*
	Generated pseudo legal moves are in the following order :
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
	int promotions_count = ((pseudo_legal_moves_indexes.queen_promotion + 1) & index_max_value) - ((pseudo_legal_moves_indexes.pawn_capture + 1) & index_max_value);
	pseudo_legal_moves_vector.resize(pseudo_legal_moves_indexes.castle + 1 + promotions_count * 3);
	int i = 0;
	while (i < ((pseudo_legal_moves_indexes.quiet_pawn + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = QUIET_PAWN;
	}
	while (i < ((pseudo_legal_moves_indexes.pawn_capture + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = CAPTURE_WITH_PAWN;
	}
	while (i < ((pseudo_legal_moves_indexes.queen_promotion + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = QUEEN_PROMOTION;
	}
	while (i < ((pseudo_legal_moves_indexes.knight_capture + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = CAPTURE_WITH_KNIGHT;
	}
	while (i < ((pseudo_legal_moves_indexes.quiet_knight + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = QUIET_KNIGHT;
	}
	while (i < ((pseudo_legal_moves_indexes.bishop_capture + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = CAPTURE_WITH_BISHOP;
	}
	while (i < ((pseudo_legal_moves_indexes.quiet_bishop + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = QUIET_BISHOP;
	}
	while (i < ((pseudo_legal_moves_indexes.rook_capture + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = CAPTURE_WITH_ROOK;
	}
	while (i < ((pseudo_legal_moves_indexes.quiet_rook + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = QUIET_ROOK;
	}
	while (i < ((pseudo_legal_moves_indexes.queen_capture + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = CAPTURE_WITH_QUEEN;
	}
	while (i < ((pseudo_legal_moves_indexes.quiet_queen + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = QUIET_QUEEN;
	}
	while (i < ((pseudo_legal_moves_indexes.king_capture + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = CAPTURE_WITH_KING;
	}
	while (i < ((pseudo_legal_moves_indexes.quiet_king + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = QUIET_KING;
	}
	while (i < ((pseudo_legal_moves_indexes.castle + 1) &index_max_value))
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[i];
		pseudo_legal_moves_vector[i++].move_type = CASTLE;
	}
	//add knight, rook, bishop promotions
	for (int j = 1; j <= promotions_count; ++j)
	{
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[((pseudo_legal_moves_indexes.pawn_capture + 1) & index_max_value) + j-1];
		pseudo_legal_moves_vector[i++].move_type = KNIGHT_PROMOTION;
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[((pseudo_legal_moves_indexes.pawn_capture + 1) & index_max_value) + j-1];
		pseudo_legal_moves_vector[i++].move_type = ROOK_PROMOTION;
		pseudo_legal_moves_vector[i].move = pseudo_legal_moves[((pseudo_legal_moves_indexes.pawn_capture + 1) & index_max_value) + j-1];
		pseudo_legal_moves_vector[i++].move_type = BISHOP_PROMOTION;
		if (((pseudo_legal_moves_indexes.pawn_capture + 1) & index_max_value) + j - 1 > 219)
			std::abort();
	}
	if (i > 219)
		std::abort();
	return pseudo_legal_moves_vector;
}