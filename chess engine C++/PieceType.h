#pragma once
#include <cstdint>

enum PieceType : uint8_t
{
	PAWN = 0,
	KNIGHT = 1,
	BISHOP = 2,
	ROOK = 3,
	QUEEN = 4,
	KING = 5,
	EMPTY = 6,
	NONE = 7
};