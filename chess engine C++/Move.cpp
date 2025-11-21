#include "PieceType.h"
#include "Move.h"
#include "MoveType.h"
#include <iostream>
#include <cassert>


Move::Move(uint16_t move, MoveType move_type)
	: move(move), move_type(move_type)
{}
Move::Move()
	: move(0), move_type(MoveType::QUIET_PAWN)
{}

bool Move::operator==(const Move& other) const
{
	if ((move >> 12) != 0 || (other.move >> 12) != 0)
	{
		std::cout << "val this: " << std::hex << move << std::endl;
		std::cout << "val other: " << std::hex << other.move << std::endl;
		std::cout << "Move high bits set! move=" << move << " other=" << other.move << std::endl;
		std::abort();
	}
	return other.move==move && other.move_type==move_type;
}

bool Move::operator<(const Move& other) const
{
	if ((move >> 12) != 0 || (other.move >> 12) != 0) {
		std::cout << "val: " << std::hex << move << std::endl;
		std::cout << "val other: " << std::hex << other.move << std::endl;
		std::cout << "Move high bits set! move=" << move << " other=" << other.move << std::endl;
		std::abort();
	}
	if (move != other.move) {
		return move < other.move;
	}
	return move_type < other.move_type;
}