#pragma once
#include "PieceType.h"
#include <cstdint>

enum MoveType : uint8_t
{//promotions may be captures
	QUEEN_PROMOTION = (0 << 3) | PAWN,
	CAPTURE_WITH_PAWN = (1 << 3) | PAWN,
	CAPTURE_WITH_KNIGHT = (2 << 3) | KNIGHT,
	CAPTURE_WITH_BISHOP = (3 << 3) | BISHOP,
	CAPTURE_WITH_ROOK = (4 << 3) | ROOK,
	CAPTURE_WITH_QUEEN = (5 << 3) | QUEEN,
	CAPTURE_WITH_KING = (6 << 3) | KING,
	CASTLE = (7 << 3) | KING,
	QUIET_KNIGHT = (8 << 3) | KNIGHT,
	QUIET_BISHOP = (9 << 3) | BISHOP,
	QUIET_ROOK = (10 << 3) | ROOK,
	QUIET_QUEEN = (11 << 3) | QUEEN,
	QUIET_KING = (12 << 3) | KING,
	QUIET_PAWN = (13 << 3) | PAWN,
	KNIGHT_PROMOTION = (14 << 3) | PAWN,
	BISHOP_PROMOTION = (15 << 3) | PAWN,
	ROOK_PROMOTION = (16 << 3) | PAWN,
	NUM_MOVE_TYPES = 17
};