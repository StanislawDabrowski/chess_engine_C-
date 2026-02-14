#pragma once
#include <cstdint>
#include "MoveGenerator.h"
#include "TTEntry.h"
#include "Board.h"
#include "SearchResult.h"
#include <limits>

typedef uint64_t Bitboard;

constexpr uint32_t floor_pow2(uint32_t x) {
	if (x == 0) return 0;
	uint32_t r = 1;
	while ((r << 1) <= x) r <<= 1;
	return r;
}

constexpr uint32_t ceil_pow2(uint32_t x) {
	if (x <= 1) return 1;
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x;
}


class Engine
{
private:
	TTEntry* tt;
	uint16_t moves_liklihood_to_be_good_scores[MoveGenerator::max_legal_moves_count];
	//ScoredMove* scored_moves_adress_plus1 = scored_moves + 1;
	/*static constexpr ScoredMove scored_move_queen_promotion = ScoredMove(Move(0, MoveType::QUEEN_PROMOTION), 0);
	static constexpr ScoredMove scored_move_rook_promotion = ScoredMove(Move(0, MoveType::ROOK_PROMOTION), 0);

	static constexpr ScoredMove scored_move_knight_promotion = ScoredMove(Move(0, MoveType::KNIGHT_PROMOTION), 0);
	static constexpr ScoredMove scored_move_bishop_promotion = ScoredMove(Move(0, MoveType::BISHOP_PROMOTION), 0);

	static constexpr ScoredMove scored_move_capture_with_pawn   = ScoredMove(Move(0, MoveType::CAPTURE_WITH_PAWN), 0);
	static constexpr ScoredMove scored_move_capture_with_knight = ScoredMove(Move(0, MoveType::CAPTURE_WITH_KNIGHT), 0);
	static constexpr ScoredMove scored_move_capture_with_bishop = ScoredMove(Move(0, MoveType::CAPTURE_WITH_BISHOP), 0);
	static constexpr ScoredMove scored_move_capture_with_rook   = ScoredMove(Move(0, MoveType::CAPTURE_WITH_ROOK), 0);
	static constexpr ScoredMove scored_move_capture_with_queen  = ScoredMove(Move(0, MoveType::CAPTURE_WITH_QUEEN), 0);
	static constexpr ScoredMove scored_move_capture_with_king   = ScoredMove(Move(0, MoveType::CAPTURE_WITH_KING), 0);

	static constexpr ScoredMove scored_move_castle               = ScoredMove(Move(0, MoveType::CASTLE), 0);

	static constexpr ScoredMove scored_move_quiet_knight        = ScoredMove(Move(0, MoveType::QUIET_KNIGHT), 0);
	static constexpr ScoredMove scored_move_quiet_bishop        = ScoredMove(Move(0, MoveType::QUIET_BISHOP), 0);
	static constexpr ScoredMove scored_move_quiet_rook          = ScoredMove(Move(0, MoveType::QUIET_ROOK), 0);
	static constexpr ScoredMove scored_move_quiet_queen         = ScoredMove(Move(0, MoveType::QUIET_QUEEN), 0);
	static constexpr ScoredMove scored_move_quiet_king          = ScoredMove(Move(0, MoveType::QUIET_KING), 0);
	static constexpr ScoredMove scored_move_quiet_pawn          = ScoredMove(Move(0, MoveType::QUIET_PAWN), 0);*/

	//multipliers are always set to power of 2 to allow bit shifting optimalization instead of multiplication - not actually necessary for most of the constants since for know all multipliers are used only in constexpr expressions so compiler will precompute it anyway
	static constexpr uint8_t capture_multiplier = 1 << 3;//4
	static constexpr uint8_t capture_thread_multiplier = 1 << 1;//2
	static constexpr uint8_t promotion_multiplier = 1 << 3;//8
	static constexpr uint16_t defended_piece_multiplier[6] = { 128, 256, 256, 512, 512 };//order: PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING


	int16_t minimax(uint8_t depth, int16_t alpha = std::numeric_limits<int16_t>::min(), int16_t beta = std::numeric_limits<int16_t>::max(), bool force_TT_entry_replacement = false);//returns score and the best move via address. Called only from find_best_move()
	int16_t quiescence_search(int16_t alpha, int16_t beta, bool force_TT_entry_replacement = false);


public:
	static constexpr uint32_t tt_size = 1UL << 27;//ceil_pow2((1<<31)/sizeof(TTEntry));


	Board* board;

	static uint64_t minmax_calls_count;
	static uint64_t quiescence_search_calls_count;


	
	Engine(Board* board);
	~Engine();

	void clear_TT();//clear the TT
	SearchResult minimax_init(uint8_t depth);//initializes minmax search, called from outside
};