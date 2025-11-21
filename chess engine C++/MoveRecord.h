#pragma once
#include <cstdint>
#include "Move.h"
#include "MoveType.h"
#include "PieceType.h"

typedef uint64_t Bitboard;


class MoveRecord
{
public:
	uint16_t move;
	MoveType move_type;
	PieceType captured_piece;//PAWN-QUEEN - taken piece; EMPTY - en passant capture; NONE - no piece taken
	uint8_t en_passant_square;
	uint8_t halfmove_clock;
	uint8_t castling_rights;
//public:
	MoveRecord() = default;
	MoveRecord(uint16_t move, MoveType move_type, uint8_t en_passant_square, uint8_t halfmove_clock, uint8_t castling_rights);

	/*Move get_move();
	int get_en_passant_square();
	int get_halfmove_clock();
	int get_from();
	int get_to();
	PieceType get_piece();
	PieceType get_captured();
	PieceType get_promotion();
	bool get_kingsite_white();
	bool get_queensite_white();
	bool get_kingsite_black();
	bool get_queensite_black();*/
};

