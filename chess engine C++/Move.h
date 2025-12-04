#pragma once
#include <cstdint>
#include "PieceType.h"
#include "MoveType.h"


class Move
{
public:
	uint16_t move;//first 6 bits: from square (0-63), next 6 bits: to square (0-63)
	MoveType move_type;

	constexpr Move(uint16_t move, MoveType move_type)
		: move(move), move_type(move_type) {
	}
	constexpr Move() = default;
	bool operator==(const Move& other) const;
	bool operator<(const Move& other) const;
};

