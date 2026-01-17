#include "MoveRecord.h"
#include "Move.h"
#include "PieceType.h"
#include <utility>


MoveRecord::MoveRecord(uint16_t move, MoveType move_type, uint8_t en_passant_square, uint8_t halfmove_clock, uint8_t castling_rights, uint16_t repetition_table_last_relevant_position)
	: move(move), move_type(move_type), en_passant_square(en_passant_square), halfmove_clock(halfmove_clock), castling_rights(castling_rights), repetition_table_last_relevant_position(repetition_table_last_relevant_position)
{ }