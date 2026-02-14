#pragma once
#include <cstdint>

class MovesIndexes16bit
{
public:
	MovesIndexes16bit();

	uint16_t quiet_pawn;
	uint16_t pawn_capture;
	uint16_t queen_promotion;//number of pawn promotions, does not count different promotion types separately (the actual moves with promotion number is 4 times larger)
	uint16_t knight_capture;
	uint16_t quiet_knight;
	uint16_t bishop_capture;
	uint16_t quiet_bishop;
	uint16_t rook_capture;
	uint16_t quiet_rook;
	uint16_t queen_capture;
	uint16_t quiet_queen;
	uint16_t king_capture;
	uint16_t quiet_king;
	uint16_t castle;
};