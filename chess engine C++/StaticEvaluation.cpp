#include <bit>
#include "StaticEvaluation.h"
#include "Board.h"
#include <cstdint>
#include "PieceType.h"
#include <cstring>

#ifdef _MSC_VER
#define ASSUME(cond) __assume(cond)
#elif defined(__clang__)
#define ASSUME(cond) __builtin_assume(cond)
#else
#define ASSUME(cond) ((void)0)
#endif


StaticEvaluation::StaticEvaluation(Board* board)
{

	this->board = board;
	

	for (int b = 0; b < 256; ++b) {
		uint64_t v = 0;
		for (int i = 0; i < 8; ++i)
			v |= uint64_t(((b >> i) & 1) ? 0xFFu : 0u) << (i * 8);
		expand8[b] = v;
	}
}

StaticEvaluation::StaticEvaluation()
{
	//do nothing
}

/*void StaticEvaluation::reset_score()
{
	score = 0;
}*/

/*int StaticEvaluation::get_score()
{
	return score;
}*/


void StaticEvaluation::calculate_score(bool pseudo_legal_moves_generated)
{
	//reset_score();
	score = 0;
	side_to_move_bonus();//needed only with quiescence search since if the is no quiescence search every position in one search is at the move of the same side which means this bonus is constant and can be ignored
	material_balance_without_pawns();
	pieces_positional_score();
	king_safety();
	pawns();
	rooks_on_open_files();
	castle_rights();
	activity(pseudo_legal_moves_generated);//deleting boosts from ~150 000 minmax calls per sec to ~250 000
	threats();//deleting boosts from ~150 000 minmax calls per sec to ~200 000
}


void inline StaticEvaluation::side_to_move_bonus()
{
	score += (board->side_to_move * 2 - 1) * (-moving_side_bonus);
}

void inline StaticEvaluation::material_balance_without_pawns()
{
	for (uint8_t piece = KNIGHT; piece < KING; ++piece)
	{
		ASSUME(std::popcount(board->P[piece][0]) <= 10);
		ASSUME(std::popcount(board->P[piece][1]) <= 10);
		score += std::popcount(board->P[piece][0]) * piece_values[piece] - std::popcount(board->P[piece][1]) * piece_values[piece];
	}
}

void inline StaticEvaluation::pieces_positional_score()
{

	ASSUME(std::popcount(board->P[PAWN][0]) <= 8);
	ASSUME(std::popcount(board->P[PAWN][1]) <= 8);

	ASSUME(std::popcount(board->P[KING][0]) == 1);
	ASSUME(std::popcount(board->P[KING][1]) == 1);
	for (char piece = PAWN; piece <= KING; ++piece) {

		ASSUME(std::popcount(board->P[piece][0]) <= 10);
		ASSUME(std::popcount(board->P[piece][1]) <= 10);
		// white pieces
		uint64_t bb = board->P[piece][0];
		unsigned long sq;

		while (bb) {
			sq = std::countr_zero(bb);    // index 0..63 of least-significant set bit
			score += pst_table_white[piece][sq];
			bb &= bb - 1;                    // clear LSB
		}

		// black pieces (mirror the square to use the same table oriented for white)
		bb = board->P[piece][1];
		while (bb) {
			sq = std::countr_zero(bb);
			score -= pst_table_black[piece][sq];
			bb &= bb - 1;
		}
	}
	
}

void inline StaticEvaluation::king_safety()
{
	int flag = 1;
	for (char c = 0; c < 2; ++c)
	{
		unsigned long ks;
		ks = std::countr_zero(board->P[KING][c]);
		for (int i = 0; i < 3; i++)
		{
			if (ks + front_offsets[c][i] < 64)
			{
				if (board->P[PAWN][c] & (1ULL << (ks + front_offsets[c][i]))) score += (i == 0 ? 20 : (i == 1 ? 10 : 5)) * flag;
			}
		}
		int ks_mod_e = ks % 8;
		if (ks_mod_e > 0)
		{
			for (int i = 0; i < 3; i++)
			{
				if (ks + diag_offsets[c][i] < 64)
				{
					if (board->P[PAWN][c] & (1ULL << (ks + diag_offsets[c][i]))) score += (i == 0 ? 10 : (i == 1 ? 5 : 2)) * flag;
				}
			}
			if (ks_mod_e < 7)
			{
				for (int i = 0; i < 3; i++)
				{
					if (ks + anti_diag_offsets[c][i] < 64)
					{
						if (board->P[PAWN][c] & (1ULL << (ks + anti_diag_offsets[c][i]))) score += (i == 0 ? 10 : (i == 1 ? 5 : 2)) * flag;
					}
				}
			}

		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				if (ks + anti_diag_offsets[c][i] < 64)
				{
					if (board->P[PAWN][c] & (1ULL << (ks + anti_diag_offsets[c][i]))) score += (i == 0 ? 10 : (i == 1 ? 5 : 2)) * flag;
				}
			}
		}
		flag = -1;
	}
}

void inline StaticEvaluation::pawns()
{
	int file_value[2][8];
	for (int c = 0; c < 2; ++c)
		for (int i = 0; i < 8; i++)
			file_value[c][i] = board->number_of_pawns_on_files[c][i] * piece_values[PAWN];

	int temp;
	for (int c = 0; c < 2; ++c)
	{
		//side support
		//non on the side pawns
		for (int i = 1; i < 7; i++)
		{
			temp = board->number_of_pawns_on_files[c][i] * 10;
			file_value[c][i] += temp * board->number_of_pawns_on_files[c][i - 1];
			file_value[c][i] += temp * board->number_of_pawns_on_files[c][i + 1];
		}
		//side pawns
		temp = board->number_of_pawns_on_files[c][0] * 10;
		file_value[c][0] += temp * board->number_of_pawns_on_files[c][1];
		temp = board->number_of_pawns_on_files[c][7] * 10;
		file_value[c][7] += temp * board->number_of_pawns_on_files[c][6];


		int opp = 1 - c;
		//free pawns
		for (int i = 0; i < 8; i++)
			if (board->number_of_pawns_on_files[opp][i] == 0)
				file_value[c][i] *= 1.25;
	}

	for (int i = 0; i < 8; i++)
		score += file_value[0][i] - file_value[1][i];

}

void inline StaticEvaluation::rooks_on_open_files()
{
	int flag = 1;
	int temp;
	for (int c = 0; c < 2; ++c)
	{
		int opp = 1 - c;
		for (int i = 0; i < 8; i++)
		{
			temp = 25 * flag;
			if (board->number_of_pawns_on_files[opp][i] == 0)
				score += std::popcount(board->P[ROOK][c] & (0x0101010101010101ULL << i)) * temp;
		}
		flag = -1;
	}
}

void inline StaticEvaluation::castle_rights()
{
	//castling rights
	//15 points for each side
	score += (board->castling_rights & 1 ? castle_right_bonus : 0);
	score += (board->castling_rights & 2 ? castle_right_bonus : 0);
	score -= (board->castling_rights & 4 ? castle_right_bonus : 0);
	score -= (board->castling_rights & 8 ? castle_right_bonus : 0);
}

void inline StaticEvaluation::activity(bool pseudo_legal_moves_generated)
{//to consider in the future: trapped pieces

	//curently taking into account:
	//mobility
	//attacks on near king squares
	//center control (embedded in const square_mobility_value)


	uint8_t opp = board->side_to_move ^ 1;
	if (!pseudo_legal_moves_generated)
		board->mg.generate_pseudo_legal_attacks(board->side_to_move);//main cost
	board->mg.generate_pseudo_legal_attacks(opp);//main cost

	//mobility
	//add 1 each index in square_mobility_value which opp king is next to and 3 for king square
	uint16_t mobility_cpy_side[64];
	uint16_t mobility_cpy_opp[64];
	std::memcpy(mobility_cpy_side, square_mobility_value+board->side_to_move, 128);
	std::memcpy(mobility_cpy_opp, square_mobility_value+opp, 128);
	unsigned long king_index_side;
	unsigned long king_index_opp;
	king_index_side = std::countr_zero(board->P[KING][opp]);
	king_index_opp = std::countr_zero(board->P[KING][board->side_to_move]);
	/*uint8_t king_index_side_mod_8 = king_index_side % 8;
	uint8_t king_index_opp_mod_8 = king_index_opp % 8;*/
	mobility_cpy_side[king_index_side] += 300;
	mobility_cpy_opp[king_index_opp] += 300;


	Bitboard squares_next_to_king_side = board->mg.king_attack_tables[king_index_side];
	Bitboard squares_next_to_king_opp = board->mg.king_attack_tables[king_index_opp];

	while (squares_next_to_king_side)
	{
		unsigned long sq;
		sq = std::countr_zero(squares_next_to_king_side);
		mobility_cpy_side[sq] += 100;
		squares_next_to_king_side &= squares_next_to_king_side - 1;
	}
	while (squares_next_to_king_opp)
	{
		unsigned long sq;
		sq = std::countr_zero(squares_next_to_king_opp);
		mobility_cpy_opp[sq] += 100;
		squares_next_to_king_opp &= squares_next_to_king_opp - 1;
	}


	int32_t mobility_score = 0;
	for (uint8_t i = 0; i < 64; ++i)
	{
		mobility_score += (mobility_cpy_side[i] * board->mg.all_attacks_count[board->side_to_move][i] - mobility_cpy_opp[i] * board->mg.all_attacks_count[opp][i])*(board->side_to_move*(-2)+1);
	}
	score += mobility_score / mobility_dividor;
	//std::cout << "activity: " << mobility_score / mobility_dividor << std::endl;

	
}


void inline StaticEvaluation::multiply_raw_array(uint8_t t[64], uint64_t m) {
	for (int block = 0; block < 8; ++block) {
		uint64_t mask = expand8[(m >> (block * 8)) & 0xFFu];
		uint64_t* chunk = reinterpret_cast<uint64_t*>(t + block * 8);
		*chunk &= mask;
	}
}

void inline StaticEvaluation::threats()
{//to consider in the furute: calculating defended pieces the same way as attacked but with different weight (adjustng the dividor)

	alignas(8) uint8_t all_attacks_count[6][2][64];

	std::memcpy(all_attacks_count[0], board->mg.all_attacks_count, sizeof(board->mg.all_attacks_count));
	std::memcpy(all_attacks_count[1], board->mg.all_attacks_count, sizeof(board->mg.all_attacks_count));
	std::memcpy(all_attacks_count[2], board->mg.all_attacks_count, sizeof(board->mg.all_attacks_count));
	std::memcpy(all_attacks_count[3], board->mg.all_attacks_count, sizeof(board->mg.all_attacks_count));
	std::memcpy(all_attacks_count[4], board->mg.all_attacks_count, sizeof(board->mg.all_attacks_count));
	std::memcpy(all_attacks_count[5], board->mg.all_attacks_count, sizeof(board->mg.all_attacks_count));

	uint16_t sums[6][2] = { 0 };


	int j;
	for (int i = 0; i < 6; i++)
	{
		multiply_raw_array(all_attacks_count[i][0], board->P[i][1]);
		multiply_raw_array(all_attacks_count[i][1], board->P[i][0]);

		//sum up
		for (j = 0; j < 64; j++)
		{
			sums[i][0] += all_attacks_count[i][0][j];
			sums[i][1] += all_attacks_count[i][1][j];
		}
	}
	int temp_score = 0;
	temp_score += (sums[PAWN][0] - sums[PAWN][1]) * piece_values[PAWN];
	temp_score += (sums[KNIGHT][0] - sums[KNIGHT][1]) * piece_values[KNIGHT];
	temp_score += (sums[BISHOP][0] - sums[BISHOP][1]) * piece_values[BISHOP];
	temp_score += (sums[ROOK][0] - sums[ROOK][1]) * piece_values[ROOK];
	temp_score += (sums[QUEEN][0] - sums[QUEEN][1]) * piece_values[QUEEN];
	temp_score += (sums[KING][0] - sums[KING][1]) * 1350;
	
	score += temp_score / threat_dividor;
	//std::cout << "threats: " << temp_score / threat_dividor << std::endl;
}