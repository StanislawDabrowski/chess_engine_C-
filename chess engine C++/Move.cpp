#include "Move.h"
#include <iostream>
#include <cstdlib>
#include <ios>





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