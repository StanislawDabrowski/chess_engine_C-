#pragma once
#include <stdint.h>

class MovesIndexes8bit
{
public:
	MovesIndexes8bit();

	uint8_t quiet_pawn;
	uint8_t pawn_capture;
	uint8_t queen_promotion;//number of pawn promotions, does not count different promotion types separately (the actual moves with promotion number is 4 times larger)
	uint8_t knight_capture;
	uint8_t quiet_knight;
	uint8_t bishop_capture;
	uint8_t quiet_bishop;
	uint8_t rook_capture;
	uint8_t quiet_rook;
	uint8_t queen_capture;
	uint8_t quiet_queen;
	uint8_t king_capture;
	uint8_t quiet_king;
	uint8_t castle;
};