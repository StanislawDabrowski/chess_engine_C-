#include <vector>
#include <iostream>
#include <sstream>
#include "Board.h"
#include "Move.h"
#include "MoveRecord.h"
#include "PieceType.h"
#include "StaticEvaluation.h"

Board::Board()
	: mg(this), se(this)
{
	//initialize every pice to 0;
	all_pieces = 0;
	for (int i = 0; i < 2; i++)
	{
		P[PAWN][i] = 0;
		P[KNIGHT][i] = 0;
		P[BISHOP][i] = 0;
		P[ROOK][i] = 0;
		P[QUEEN][i] = 0;
		P[KING][i] = 0;
		all_pieces_types[i] = 0;
	}

	for (int i = 0; i < 8; i++)
	{
		number_of_pawns_on_files[0][i] = 0;
		number_of_pawns_on_files[1][i] = 0;
	}

	castling_rights = 0b0000;


	side_to_move = 0;
	en_passant_square = 0;
	halfmove_clock = 0;


	initialize_castling_mask();
}


void Board::initialize_castling_mask()
{
	//initialize the castling mask
	for (int i = 0; i < 64; i++)
	{
		castling_mask[i] = 0b1111;
	}
	//white queensite
	castling_mask[0] = 0b1101;
	//white kingsite
	castling_mask[7] = 0b1110;
	//white king
	castling_mask[4] = 0b1100;
	//black queensite
	castling_mask[56] = 0b0111;
	//black kingsite
	castling_mask[63] = 0b1011;
	//black king
	castling_mask[60] = 0b0011;
}

void Board::set_piece_on_square(uint8_t square, PieceType piece, uint8_t side)
{
	P[piece][side] |= 1ULL << square;
	//update all pieces bitboards
	all_pieces_types[side] |= 1ULL << square;
	all_pieces = all_pieces_types[0] | all_pieces_types[1];
	if (piece==PAWN)
	{
		number_of_pawns_on_files[side][square % 8]++;
	}
}

bool Board::set_piece_fen(char c, int sq)
{
	//c - piece character from FEN
	//can be 'P','N','B','R','Q','K' for white pieces and 'p','n','b','r','q','k' for black pieces
	//use set_piece_on_square to set the piece on the square
	uint8_t side;
	if (std::isupper(static_cast<unsigned char>(c)))
		side = 0;
	else
		side = 1;
	switch (std::tolower(static_cast<unsigned char>(c)))
	{
	case 'p':
		set_piece_on_square(sq, PAWN, side);//increments file couts if it's pawn move
		//number_of_pawns_on_files[side][sq % 8]++;
		break;
	case 'n':
		set_piece_on_square(sq, KNIGHT, side);
		break;
	case 'b':
		set_piece_on_square(sq, BISHOP, side);
		break;
	case 'r':
		set_piece_on_square(sq, ROOK, side);
		break;
	case 'q':
		set_piece_on_square(sq, QUEEN, side);
		break;
	case 'k':
		set_piece_on_square(sq, KING, side);
		break;
	default:
		return false;
	}
	return true;
}

void Board::initialize_board()
{
	//initialize the board to the starting position


	P[PAWN][0] = 0x000000000000FF00;
	P[KNIGHT][0] = 0x0000000000000042;
	P[BISHOP][0] = 0x0000000000000024;
	P[ROOK][0] = 0x0000000000000081;
	P[QUEEN][0] = 0x0000000000000008;
	P[KING][0] = 0x0000000000000010;
	P[PAWN][1] = 0x00FF000000000000;
	P[KNIGHT][1] = 0x4200000000000000;
	P[BISHOP][1] = 0x2400000000000000;
	P[ROOK][1] = 0x8100000000000000;
	P[QUEEN][1] = 0x0800000000000000;
	P[KING][1] = 0x1000000000000000;
	all_pieces_types[0] = P[PAWN][0] | P[KNIGHT][0] | P[BISHOP][0] | P[ROOK][0] | P[QUEEN][0] | P[KING][0];
	all_pieces_types[1] = P[PAWN][1] | P[KNIGHT][1] | P[BISHOP][1] | P[ROOK][1] | P[QUEEN][1] | P[KING][1];
	all_pieces = all_pieces_types[0] | all_pieces_types[1];

	for (int i = 0; i < 8; i++)
	{
		number_of_pawns_on_files[0][i] = 1;
		number_of_pawns_on_files[1][i] = 1;
	}

	castling_rights = 0b1111;


	side_to_move = 0;
	en_passant_square = 0;
	halfmove_clock = 0;

	//pawns on files
	/*for (int i = 0; i < 8; i++)
	{
		number_of_pawns_on_files[0][i] = 0;
		number_of_pawns_on_files[1][i] = 0;
	}*/

	//moves stack
	moves_stack_size = 0;

	//all pieces bitboards
	all_pieces_types[0] = P[PAWN][0] | P[KNIGHT][0] | P[BISHOP][0] | P[ROOK][0] | P[QUEEN][0] | P[KING][0];
	all_pieces_types[1] = P[PAWN][1] | P[KNIGHT][1] | P[BISHOP][1] | P[ROOK][1] | P[QUEEN][1] | P[KING][1];
	all_pieces = all_pieces_types[0] | all_pieces_types[1];
	calculate_zobrist_key();
}

bool Board::load_fen(const std::string& fen)
{
	// reset storage
	for (int t = 0; t < 6; ++t) {
		P[t][0] = 0ULL;
		P[t][1] = 0ULL;
	}
	all_pieces_types[0] = all_pieces_types[1] = 0ULL;
	all_pieces = 0ULL;
	castling_rights = 0;
	//for (int i = 0; i < 64; ++i) castling_mask[i] = 0;//have no idea we it even was here, on 95% it absolutly should not be here
	for (int s = 0; s < 8; ++s) {
		number_of_pawns_on_files[0][s] = 0;
		number_of_pawns_on_files[1][s] = 0;
	}
	moves_stack_size = 0;
	en_passant_square = 0;
	halfmove_clock = 0;
	side_to_move = 0;


	std::istringstream ss(fen);
	std::string piece_placement, active_color, castling, enpassant, halfmove_s, fullmove_s;
	if (!(ss >> piece_placement)) return false;
	if (!(ss >> active_color)) return false;
	if (!(ss >> castling)) return false;
	if (!(ss >> enpassant)) return false;
	if (!(ss >> halfmove_s)) return false;
	if (!(ss >> fullmove_s)) {
		// fullmove is optional for our usage; still try to proceed
		fullmove_s = "1";
	}
	int sq = 0;
	int rank = 7; // start at rank 8 (index 7)
	int file = 0;
	size_t i = 0;
	while (i < piece_placement.size())
	{
		if (piece_placement[i] == '/') {
			if (file != 8) return false;
			--rank;
			file = 0;
			++i;
			continue;
		}
		char c = piece_placement[i];
		if (std::isdigit(static_cast<unsigned char>(c))) {
			int empties = c - '0';
			if (empties < 1 || empties > 8) return false;
			file += empties;
			if (file > 8) return false;
		}
		else {
			if (file >= 8) return false;
			int sq_index = rank * 8 + file; // mapping 0=a1
			if (!set_piece_fen(c, sq_index)) return false;
			++file;
		}
		++i;
	}
	if (rank != 0 || file != 8) {
		// final rank must end exactly at file 8 and rank 1 processed
		// note: some FENs end with rank==0 and file==8
		// Accept only exact complete placement
		// If rank>0 we didn't parse all ranks
		// If file!=8 final rank incomplete
		// Both indicate malformed FEN
		// Return false
		return false;
	}
	if (active_color.size() != 1) return false;
	if (active_color[0] == 'w') side_to_move = 0;
	else if (active_color[0] == 'b') side_to_move = 1;
	else return false;


	castling_rights = 0;
	if (castling != "-") {
		for (char c : castling) {
			switch (c) {
			case 'K': castling_rights |= (1u << 0); break; // white kingside
			case 'Q': castling_rights |= (1u << 1); break; // white queenside
			case 'k': castling_rights |= (1u << 2); break; // black kingside
			case 'q': castling_rights |= (1u << 3); break; // black queenside
			default: return false;
			}
		}
	}

	// en passant square
	if (enpassant == "-") {
		en_passant_square = 0;
	}
	else {
		if (enpassant.size() != 2) return false;
		char f = enpassant[0];
		char r = enpassant[1];
		if (f < 'a' || f > 'h' || r < '1' || r > '8') return false;
		int file_ep = f - 'a';
		int rank_ep = (r - '1'); // 0..7
		en_passant_square = rank_ep * 8 + file_ep;
	}

	// halfmove clock
	try {
		halfmove_clock = std::stoi(halfmove_s);
	}
	catch (...) {
		return false;
	}
	calculate_zobrist_key();
	return true;

}

void Board::calculate_zobrist_key()
{
	//init zobrist key
	zobrist_key = 0;
	//calculate initial zobrist key
	unsigned long square;
	for (int color = 0; color < 2; color++)
	{
		for (int piece = 0; piece < 6; piece++)
		{
			Bitboard pieces = P[piece][color];
			while (pieces)
			{

				_BitScanForward64(&square, pieces);
				zobrist_key ^= zobrist_pieces[color][piece][square];
				pieces &= pieces - 1;
			}
		}
	}
	//castling rights
	if (castling_rights & 1) zobrist_key ^= zobrist_castling[0];
	if (castling_rights & 2) zobrist_key ^= zobrist_castling[1];
	if (castling_rights & 4) zobrist_key ^= zobrist_castling[2];
	if (castling_rights & 8) zobrist_key ^= zobrist_castling[3];
	//en passant
	zobrist_key ^= zobrist_en_passant[en_passant_square];//if no en passant, en_passant==0 and the zobrist table at inedx 0 has 0 so it has no effect
	if (side_to_move)
		zobrist_key ^= zobrist_side_to_move;
}


void Board::make_move(SimpleMove move, MoveType move_type)
{
	Bitboard from_mask = mg.from_mask[move];
	Bitboard to_mask = mg.to_mask[move];
	Bitboard from_to_mask = mg.from_to_mask[move];
	uint8_t opp = side_to_move ^ 1;
	int from = move & 0b111111;
	int to = move >> 6;
	PieceType piece_moved = static_cast<PieceType>(move_type & 0b111);

	moves_stack[moves_stack_size].move = move;
	moves_stack[moves_stack_size].move_type = move_type;
	moves_stack[moves_stack_size].en_passant_square = en_passant_square;
	moves_stack[moves_stack_size].halfmove_clock = halfmove_clock;
	moves_stack[moves_stack_size].castling_rights = castling_rights;

	//remove the piece from the from square
	//P[move_type & 0b111][side_to_move] ^= from_mask;
	all_pieces_types[side_to_move] ^= from_to_mask;
	//update zobrist key
	zobrist_key ^= zobrist_pieces[side_to_move][piece_moved][from];
	zobrist_key ^= zobrist_pieces[side_to_move][piece_moved][to];
	zobrist_key ^= zobrist_en_passant[en_passant_square];//remove old en passant square from zobrist key

	en_passant_square = 0;//reset en passant square

	//add piece to the to square
	switch (move_type)
	{
	case QUEEN_PROMOTION://promotoins may be captures
	{
		P[QUEEN][side_to_move] |= to_mask;
		P[PAWN][side_to_move] ^= from_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[side_to_move][QUEEN][to];
		zobrist_key ^= zobrist_pieces[side_to_move][PAWN][to];
		if (all_pieces & to_mask)//capture
		{
			PieceType captured_piece;
			//determine captured piece type, no pawn capture possible on promotion square
			if (P[KNIGHT][opp] & to_mask)
				captured_piece = KNIGHT;
			else if (P[BISHOP][opp] & to_mask)
				captured_piece = BISHOP;
			else if (P[ROOK][opp] & to_mask)
			{
				captured_piece = ROOK;
				uint8_t castling_rights_old = castling_rights;
				castling_rights &= castling_mask[to];
				uint8_t castling_change = castling_rights_old ^ castling_rights;
				unsigned long bit_idx;
				while (castling_change)
				{
					_BitScanForward64(&bit_idx, castling_change);
					zobrist_key ^= zobrist_castling[bit_idx];
					castling_change &= castling_change - 1;
				}

			}
			else if (P[QUEEN][opp] & to_mask)
				captured_piece = QUEEN;
			moves_stack[moves_stack_size].captured_piece = captured_piece;
			P[captured_piece][opp] ^= to_mask;
			all_pieces_types[opp] ^= to_mask;
			all_pieces ^= from_mask;
			//update sobrist key
			zobrist_key ^= zobrist_pieces[opp][captured_piece][to];
			//update pawn count on files
			--number_of_pawns_on_files[side_to_move][from % 8];
			break;
		}
		moves_stack[moves_stack_size].captured_piece = NONE;
		//no capture
		all_pieces ^= from_to_mask;
		//update pawn count on files
		--number_of_pawns_on_files[side_to_move][from % 8];
		halfmove_clock = 0;
		break;
	}
	case KNIGHT_PROMOTION:
	{
		P[KNIGHT][side_to_move] |= to_mask;
		P[PAWN][side_to_move] ^= from_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[side_to_move][KNIGHT][to];
		zobrist_key ^= zobrist_pieces[side_to_move][PAWN][to];
		if (all_pieces & to_mask)//capture
		{
			PieceType captured_piece;
			//determine captured piece type, no pawn capture possible on promotion square
			if (P[KNIGHT][opp] & to_mask)
				captured_piece = KNIGHT;
			else if (P[BISHOP][opp] & to_mask)
				captured_piece = BISHOP;
			else if (P[ROOK][opp] & to_mask)
			{
				captured_piece = ROOK;
				uint8_t castling_rights_old = castling_rights;
				castling_rights &= castling_mask[to];
				uint8_t castling_change = castling_rights_old ^ castling_rights;
				unsigned long bit_idx;
				while (castling_change)
				{
					_BitScanForward64(&bit_idx, castling_change);
					zobrist_key ^= zobrist_castling[bit_idx];
					castling_change &= castling_change - 1;
				}
			}
			else if (P[QUEEN][opp] & to_mask)
				captured_piece = QUEEN;
			moves_stack[moves_stack_size].captured_piece = captured_piece;
			P[captured_piece][opp] ^= to_mask;
			all_pieces_types[opp] ^= to_mask;
			all_pieces ^= from_mask;
			//update sobrist key
			zobrist_key ^= zobrist_pieces[opp][captured_piece][to];
			//update pawn count on files
			--number_of_pawns_on_files[side_to_move][from % 8];
			break;
		}
		moves_stack[moves_stack_size].captured_piece = NONE;
		//no capture
		all_pieces ^= from_to_mask;
		//update pawn count on files
		--number_of_pawns_on_files[side_to_move][from % 8];
		halfmove_clock = 0;
		break;
	}
	case BISHOP_PROMOTION:
	{
		P[BISHOP][side_to_move] |= to_mask;
		P[PAWN][side_to_move] ^= from_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[side_to_move][BISHOP][to];
		zobrist_key ^= zobrist_pieces[side_to_move][PAWN][to];
		if (all_pieces & to_mask)//capture
		{
			PieceType captured_piece;
			//determine captured piece type, no pawn capture possible on promotion square
			if (P[KNIGHT][opp] & to_mask)
				captured_piece = KNIGHT;
			else if (P[BISHOP][opp] & to_mask)
				captured_piece = BISHOP;
			else if (P[ROOK][opp] & to_mask)
			{
				captured_piece = ROOK;
				uint8_t castling_rights_old = castling_rights;
				castling_rights &= castling_mask[to];
				uint8_t castling_change = castling_rights_old ^ castling_rights;
				unsigned long bit_idx;
				while (castling_change)
				{
					_BitScanForward64(&bit_idx, castling_change);
					zobrist_key ^= zobrist_castling[bit_idx];
					castling_change &= castling_change - 1;
				}
			}
			else if (P[QUEEN][opp] & to_mask)
				captured_piece = QUEEN;
			moves_stack[moves_stack_size].captured_piece = captured_piece;
			P[captured_piece][opp] ^= to_mask;
			all_pieces_types[opp] ^= to_mask;
			all_pieces ^= from_mask;
			//update sobrist key
			zobrist_key ^= zobrist_pieces[opp][captured_piece][to];
			//update pawn count on files
			--number_of_pawns_on_files[side_to_move][from % 8];
			break;
		}
		moves_stack[moves_stack_size].captured_piece = NONE;
		//no capture
		all_pieces ^= from_to_mask;
		//update pawn count on files
		--number_of_pawns_on_files[side_to_move][from % 8];
		halfmove_clock = 0;
		break;
	}
	case ROOK_PROMOTION:
	{
		P[ROOK][side_to_move] |= to_mask;
		P[PAWN][side_to_move] ^= from_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[side_to_move][ROOK][to];
		zobrist_key ^= zobrist_pieces[side_to_move][PAWN][to];//removed pawn since it was "added" before
		if (all_pieces & to_mask)//capture
		{
			PieceType captured_piece;
			//determine captured piece type, no pawn capture possible on promotion square
			if (P[KNIGHT][opp] & to_mask)
				captured_piece = KNIGHT;
			else if (P[BISHOP][opp] & to_mask)
				captured_piece = BISHOP;
			else if (P[ROOK][opp] & to_mask)
			{
				captured_piece = ROOK;
				uint8_t castling_rights_old = castling_rights;
				castling_rights &= castling_mask[to];
				uint8_t castling_change = castling_rights_old ^ castling_rights;
				unsigned long bit_idx;
				while (castling_change)
				{
					_BitScanForward64(&bit_idx, castling_change);
					zobrist_key ^= zobrist_castling[bit_idx];
					castling_change &= castling_change - 1;
				}
			}
			else if (P[QUEEN][opp] & to_mask)
				captured_piece = QUEEN;
			moves_stack[moves_stack_size].captured_piece = captured_piece;
			P[captured_piece][opp] ^= to_mask;
			all_pieces_types[opp] ^= to_mask;
			all_pieces ^= from_mask;
			//update sobrist key
			zobrist_key ^= zobrist_pieces[opp][captured_piece][to];
			//update pawn count on files
			--number_of_pawns_on_files[side_to_move][from % 8];
			break;
		}
		moves_stack[moves_stack_size].captured_piece = NONE;
		//no capture
		all_pieces ^= from_to_mask;
		//update pawn count on files
		--number_of_pawns_on_files[side_to_move][from % 8];
		halfmove_clock = 0;
		break;
	}
	case CASTLE:
	{
		moves_stack[moves_stack_size].captured_piece = NONE;
		P[KING][side_to_move] ^= from_to_mask;
		//update zobrist key
		all_pieces ^= from_to_mask;
		//handle the rook
		if (to == 6 || to == 62) //kingside castling
		{
			int rook_from = to + 1;
			int rook_to = to - 1;
			Bitboard rook_from_to_mask = (1ULL << rook_from) | (1ULL << rook_to);
			P[ROOK][side_to_move] ^= rook_from_to_mask;
			all_pieces ^= rook_from_to_mask;
			all_pieces_types[side_to_move] ^= rook_from_to_mask;
			//update zobrist key
			zobrist_key ^= zobrist_pieces[side_to_move][ROOK][rook_from];
			zobrist_key ^= zobrist_pieces[side_to_move][ROOK][rook_to];
		}
		else if (to == 2 || to == 58) //queenside castling
		{
			int rook_from = (to - 2);
			int rook_to = (to + 1);
			Bitboard rook_from_to_mask = (1ULL << rook_from) | (1ULL << rook_to);
			P[ROOK][side_to_move] ^= rook_from_to_mask;
			all_pieces ^= rook_from_to_mask;
			all_pieces_types[side_to_move] ^= rook_from_to_mask;
			//update zobrist key
			zobrist_key ^= zobrist_pieces[side_to_move][ROOK][rook_from];
			zobrist_key ^= zobrist_pieces[side_to_move][ROOK][rook_to];
		}

		if (side_to_move == 0)
		{
			uint8_t castling_rights_old = castling_rights;
			castling_rights &= 0b1100;
			uint8_t castling_change = castling_rights_old ^ castling_rights;
			unsigned long bit_idx;
			while (castling_change)
			{
				_BitScanForward64(&bit_idx, castling_change);
				zobrist_key ^= zobrist_castling[bit_idx];
				castling_change &= castling_change - 1;
			}
		}
		else
		{
			uint8_t castling_rights_old = castling_rights;
			castling_rights &= 0b0011;
			uint8_t castling_change = castling_rights_old ^ castling_rights;
			unsigned long bit_idx;
			while (castling_change)
			{
				_BitScanForward64(&bit_idx, castling_change);
				zobrist_key ^= zobrist_castling[bit_idx];
				castling_change &= castling_change - 1;
			}
		}
		++halfmove_clock;
		break;
	}
	case CAPTURE_WITH_PAWN://no promotion
	{
		PieceType captured_piece;
		//determine captured piece type
		if (P[PAWN][opp] & to_mask)
			captured_piece = PAWN;
		else if (P[KNIGHT][opp] & to_mask)
			captured_piece = KNIGHT;
		else if (P[BISHOP][opp] & to_mask)
			captured_piece = BISHOP;
		else if (P[ROOK][opp] & to_mask)
		{
			captured_piece = ROOK;
			castling_rights &= castling_mask[to];
		}
		else if (P[QUEEN][opp] & to_mask)
			captured_piece = QUEEN;
		else//king cannot be captured therefore this is en passant capture
		{
			moves_stack[moves_stack_size].captured_piece = EMPTY;//EMPTY means en passant capture

			Bitboard en_passant_pawn_mask;
			if (side_to_move == 0)
			{
				en_passant_pawn_mask = to_mask >> 8;
				P[PAWN][1] ^= en_passant_pawn_mask;

				zobrist_key ^= zobrist_pieces[1][PAWN][to - 8];
			}
			else
			{
				en_passant_pawn_mask = to_mask << 8;
				P[PAWN][0] ^= en_passant_pawn_mask;

				zobrist_key ^= zobrist_pieces[0][PAWN][to + 8];
			}
			P[PAWN][side_to_move] ^= from_to_mask;
			all_pieces_types[opp] ^= en_passant_pawn_mask;
			all_pieces ^= en_passant_pawn_mask | from_to_mask;
			//update pawn count on files
			int to_file = to % 8;
			--number_of_pawns_on_files[opp][to_file];
			--number_of_pawns_on_files[side_to_move][from % 8];
			++number_of_pawns_on_files[side_to_move][to_file];
			halfmove_clock = 0;
			break;
		}
		moves_stack[moves_stack_size].captured_piece = captured_piece;
		P[PAWN][side_to_move] ^= from_to_mask;
		P[captured_piece][opp] ^= to_mask;
		all_pieces_types[opp] ^= to_mask;
		all_pieces ^= from_mask;
		//update sobrist key
		zobrist_key ^= zobrist_pieces[opp][captured_piece][to];
		//update pawn count on files
		int to_file = to % 8;
		--number_of_pawns_on_files[side_to_move][from % 8];
		++number_of_pawns_on_files[side_to_move][to_file];
		if (captured_piece == PAWN)
			--number_of_pawns_on_files[opp][to_file];
		break;
	}
	case CAPTURE_WITH_ROOK:
	{
		//update castling rights
		uint8_t castling_rights_old = castling_rights;
		castling_rights &= castling_mask[to];
		uint8_t castling_change = castling_rights_old ^ castling_rights;
		unsigned long bit_idx;
		while (castling_change)
		{
			_BitScanForward64(&bit_idx, castling_change);
			zobrist_key ^= zobrist_castling[bit_idx];
			castling_change &= castling_change - 1;
		}
	}
	case CAPTURE_WITH_KNIGHT:
	case CAPTURE_WITH_BISHOP:
	case CAPTURE_WITH_QUEEN:
	{
		PieceType captured_piece;
		//determine captured piece type
		if (P[PAWN][opp] & to_mask)
			captured_piece = PAWN;
		else if (P[KNIGHT][opp] & to_mask)
			captured_piece = KNIGHT;
		else if (P[BISHOP][opp] & to_mask)
			captured_piece = BISHOP;
		else if (P[ROOK][opp] & to_mask)
			captured_piece = ROOK;
		else if (P[QUEEN][opp] & to_mask)
			captured_piece = QUEEN;
		moves_stack[moves_stack_size].captured_piece = captured_piece;
		P[piece_moved][side_to_move] ^= from_to_mask;
		P[captured_piece][opp] ^= to_mask;
		all_pieces_types[opp] ^= to_mask;
		all_pieces ^= from_mask;
		if (captured_piece == PAWN)
			--number_of_pawns_on_files[opp][to % 8];

		//update zobrist key
		zobrist_key ^= zobrist_pieces[opp][captured_piece][to];


		uint8_t castling_rights_old = castling_rights;
		castling_rights &= castling_mask[to];
		uint8_t castling_change = castling_rights_old ^ castling_rights;
		unsigned long bit_idx;
		while (castling_change)
		{
			_BitScanForward64(&bit_idx, castling_change);
			zobrist_key ^= zobrist_castling[bit_idx];
			castling_change &= castling_change - 1;
		}
		halfmove_clock = 0;
		break;
	}
	case CAPTURE_WITH_KING:
	{
		PieceType captured_piece;
		//determine captured piece type
		if (P[PAWN][opp] & to_mask)
			captured_piece = PAWN;
		else if (P[KNIGHT][opp] & to_mask)
			captured_piece = KNIGHT;
		else if (P[BISHOP][opp] & to_mask)
			captured_piece = BISHOP;
		else if (P[ROOK][opp] & to_mask)
			captured_piece = ROOK;
		else if (P[QUEEN][opp] & to_mask)
			captured_piece = QUEEN;
		moves_stack[moves_stack_size].captured_piece = captured_piece;
		P[KING][side_to_move] ^= from_to_mask;
		P[captured_piece][opp] ^= to_mask;
		all_pieces_types[opp] ^= to_mask;
		all_pieces ^= from_mask;
		if (captured_piece == PAWN)
			--number_of_pawns_on_files[opp][to % 8];
		//update sobrist key
		zobrist_key ^= zobrist_pieces[opp][captured_piece][to];


		uint8_t castling_rights_old = castling_rights;
		castling_rights &= castling_mask[from];
		castling_rights &= castling_mask[to];

		uint8_t castling_change = castling_rights_old ^ castling_rights;
		unsigned long bit_idx;
		while (castling_change)
		{
			_BitScanForward64(&bit_idx, castling_change);
			zobrist_key ^= zobrist_castling[bit_idx];
			castling_change &= castling_change - 1;
		}
		halfmove_clock = 0;
		break;
	}
	case QUIET_ROOK:
	{
		//update castling rights
		uint8_t castling_rights_old = castling_rights;
		castling_rights &= castling_mask[from];
		uint8_t castling_change = castling_rights_old ^ castling_rights;
		unsigned long bit_idx;
		while (castling_change)
		{
			_BitScanForward64(&bit_idx, castling_change);
			zobrist_key ^= zobrist_castling[bit_idx];
			castling_change &= castling_change - 1;
		}
	}
	case QUIET_KNIGHT:
	case QUIET_BISHOP:
	case QUIET_QUEEN:
	{
		moves_stack[moves_stack_size].captured_piece = NONE;
		P[piece_moved][side_to_move] ^= from_to_mask;
		all_pieces ^= from_to_mask;
		++halfmove_clock;
		break;
	}
	case QUIET_PAWN:
	{
		moves_stack[moves_stack_size].captured_piece = NONE;
		P[PAWN][side_to_move] ^= from_to_mask;
		all_pieces ^= from_to_mask;
		halfmove_clock = 0;
		if ((from - to == 16) || (to - from == 16))//double push
		{
			en_passant_square = (from + to) >> 1;
		}
		break;
	}
	case QUIET_KING:
	{
		moves_stack[moves_stack_size].captured_piece = NONE;
		P[KING][side_to_move] ^= from_to_mask;
		all_pieces ^= from_to_mask;


		uint8_t castling_rights_old = castling_rights;
		castling_rights &= castling_mask[from];
		castling_rights &= castling_mask[to];

		uint8_t castling_change = castling_rights_old ^ castling_rights;
		unsigned long bit_idx;
		while (castling_change)
		{
			_BitScanForward64(&bit_idx, castling_change);
			zobrist_key ^= zobrist_castling[bit_idx];
			castling_change &= castling_change - 1;
		}
		++halfmove_clock;

		break;
	}
	}
	zobrist_key ^= zobrist_en_passant[en_passant_square];

	++moves_stack_size;

	side_to_move = opp;
	zobrist_key ^= zobrist_side_to_move;

	
}

void Board::make_move(Move move)
{
	Bitboard from_mask = mg.from_mask[move.move];
	Bitboard to_mask = mg.to_mask[move.move];
	Bitboard from_to_mask = mg.from_to_mask[move.move];
	uint8_t opp = side_to_move ^ 1;
	int from = move.move & 0b111111;
	int to = move.move >> 6;
	PieceType piece_moved = static_cast<PieceType>(move.move_type & 0b111);

	moves_stack[moves_stack_size].move = move.move;
	moves_stack[moves_stack_size].move_type = move.move_type;
	moves_stack[moves_stack_size].en_passant_square = en_passant_square;
	moves_stack[moves_stack_size].halfmove_clock = halfmove_clock;
	moves_stack[moves_stack_size].castling_rights = castling_rights;

	//remove the piece from the from square
	//P[move_type & 0b111][side_to_move] ^= from_mask;
	all_pieces_types[side_to_move] ^= from_to_mask;
	//update zobrist key
	zobrist_key ^= zobrist_pieces[side_to_move][piece_moved][from];
	zobrist_key ^= zobrist_pieces[side_to_move][piece_moved][to];
	zobrist_key ^= zobrist_en_passant[en_passant_square];//remove old en passant square from zobrist key

	en_passant_square = 0;//reset en passant square

	//add piece to the to square
	switch (move.move_type)
	{
	case QUEEN_PROMOTION://promotoins may be captures
	{
		P[QUEEN][side_to_move] |= to_mask;
		P[PAWN][side_to_move] ^= from_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[side_to_move][QUEEN][to];
		zobrist_key ^= zobrist_pieces[side_to_move][PAWN][to];
		if (all_pieces & to_mask)//capture
		{
			PieceType captured_piece;
			//determine captured piece type, no pawn capture possible on promotion square
			if (P[KNIGHT][opp] & to_mask)
				captured_piece = KNIGHT;
			else if (P[BISHOP][opp] & to_mask)
				captured_piece = BISHOP;
			else if (P[ROOK][opp] & to_mask)
			{
				captured_piece = ROOK;
				uint8_t castling_rights_old = castling_rights;
				castling_rights &= castling_mask[to];
				uint8_t castling_change = castling_rights_old ^ castling_rights;
				unsigned long bit_idx;
				while (castling_change)
				{
					_BitScanForward64(&bit_idx, castling_change);
					zobrist_key ^= zobrist_castling[bit_idx];
					castling_change &= castling_change - 1;
				}

			}
			else if (P[QUEEN][opp] & to_mask)
				captured_piece = QUEEN;
			moves_stack[moves_stack_size].captured_piece = captured_piece;
			P[captured_piece][opp] ^= to_mask;
			all_pieces_types[opp] ^= to_mask;
			all_pieces ^= from_mask;
			//update sobrist key
			zobrist_key ^= zobrist_pieces[opp][captured_piece][to];
			//update pawn count on files
			--number_of_pawns_on_files[side_to_move][from % 8];
			break;
		}
		moves_stack[moves_stack_size].captured_piece = NONE;
		//no capture
		all_pieces ^= from_to_mask;
		//update pawn count on files
		--number_of_pawns_on_files[side_to_move][from % 8];
		halfmove_clock = 0;
		break;
	}
	case KNIGHT_PROMOTION:
	{
		P[KNIGHT][side_to_move] |= to_mask;
		P[PAWN][side_to_move] ^= from_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[side_to_move][KNIGHT][to];
		zobrist_key ^= zobrist_pieces[side_to_move][PAWN][to];
		if (all_pieces & to_mask)//capture
		{
			PieceType captured_piece;
			//determine captured piece type, no pawn capture possible on promotion square
			if (P[KNIGHT][opp] & to_mask)
				captured_piece = KNIGHT;
			else if (P[BISHOP][opp] & to_mask)
				captured_piece = BISHOP;
			else if (P[ROOK][opp] & to_mask)
			{
				captured_piece = ROOK;
				uint8_t castling_rights_old = castling_rights;
				castling_rights &= castling_mask[to];
				uint8_t castling_change = castling_rights_old ^ castling_rights;
				unsigned long bit_idx;
				while (castling_change)
				{
					_BitScanForward64(&bit_idx, castling_change);
					zobrist_key ^= zobrist_castling[bit_idx];
					castling_change &= castling_change - 1;
				}
			}
			else if (P[QUEEN][opp] & to_mask)
				captured_piece = QUEEN;
			moves_stack[moves_stack_size].captured_piece = captured_piece;
			P[captured_piece][opp] ^= to_mask;
			all_pieces_types[opp] ^= to_mask;
			all_pieces ^= from_mask;
			//update sobrist key
			zobrist_key ^= zobrist_pieces[opp][captured_piece][to];
			//update pawn count on files
			--number_of_pawns_on_files[side_to_move][from % 8];
			break;
		}
		moves_stack[moves_stack_size].captured_piece = NONE;
		//no capture
		all_pieces ^= from_to_mask;
		//update pawn count on files
		--number_of_pawns_on_files[side_to_move][from % 8];
		halfmove_clock = 0;
		break;
	}
	case BISHOP_PROMOTION:
	{
		P[BISHOP][side_to_move] |= to_mask;
		P[PAWN][side_to_move] ^= from_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[side_to_move][BISHOP][to];
		zobrist_key ^= zobrist_pieces[side_to_move][PAWN][to];
		if (all_pieces & to_mask)//capture
		{
			PieceType captured_piece;
			//determine captured piece type, no pawn capture possible on promotion square
			if (P[KNIGHT][opp] & to_mask)
				captured_piece = KNIGHT;
			else if (P[BISHOP][opp] & to_mask)
				captured_piece = BISHOP;
			else if (P[ROOK][opp] & to_mask)
			{
				captured_piece = ROOK;
				uint8_t castling_rights_old = castling_rights;
				castling_rights &= castling_mask[to];
				uint8_t castling_change = castling_rights_old ^ castling_rights;
				unsigned long bit_idx;
				while (castling_change)
				{
					_BitScanForward64(&bit_idx, castling_change);
					zobrist_key ^= zobrist_castling[bit_idx];
					castling_change &= castling_change - 1;
				}
			}
			else if (P[QUEEN][opp] & to_mask)
				captured_piece = QUEEN;
			moves_stack[moves_stack_size].captured_piece = captured_piece;
			P[captured_piece][opp] ^= to_mask;
			all_pieces_types[opp] ^= to_mask;
			all_pieces ^= from_mask;
			//update sobrist key
			zobrist_key ^= zobrist_pieces[opp][captured_piece][to];
			//update pawn count on files
			--number_of_pawns_on_files[side_to_move][from % 8];
			break;
		}
		moves_stack[moves_stack_size].captured_piece = NONE;
		//no capture
		all_pieces ^= from_to_mask;
		//update pawn count on files
		--number_of_pawns_on_files[side_to_move][from % 8];
		halfmove_clock = 0;
		break;
	}
	case ROOK_PROMOTION:
	{
		P[ROOK][side_to_move] |= to_mask;
		P[PAWN][side_to_move] ^= from_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[side_to_move][ROOK][to];
		zobrist_key ^= zobrist_pieces[side_to_move][PAWN][to];//removed pawn since it was "added" before
		if (all_pieces & to_mask)//capture
		{
			PieceType captured_piece;
			//determine captured piece type, no pawn capture possible on promotion square
			if (P[KNIGHT][opp] & to_mask)
				captured_piece = KNIGHT;
			else if (P[BISHOP][opp] & to_mask)
				captured_piece = BISHOP;
			else if (P[ROOK][opp] & to_mask)
			{
				captured_piece = ROOK;
				uint8_t castling_rights_old = castling_rights;
				castling_rights &= castling_mask[to];
				uint8_t castling_change = castling_rights_old ^ castling_rights;
				unsigned long bit_idx;
				while (castling_change)
				{
					_BitScanForward64(&bit_idx, castling_change);
					zobrist_key ^= zobrist_castling[bit_idx];
					castling_change &= castling_change - 1;
				}
			}
			else if (P[QUEEN][opp] & to_mask)
				captured_piece = QUEEN;
			moves_stack[moves_stack_size].captured_piece = captured_piece;
			P[captured_piece][opp] ^= to_mask;
			all_pieces_types[opp] ^= to_mask;
			all_pieces ^= from_mask;
			//update sobrist key
			zobrist_key ^= zobrist_pieces[opp][captured_piece][to];
			//update pawn count on files
			--number_of_pawns_on_files[side_to_move][from % 8];
			break;
		}
		moves_stack[moves_stack_size].captured_piece = NONE;
		//no capture
		all_pieces ^= from_to_mask;
		//update pawn count on files
		--number_of_pawns_on_files[side_to_move][from % 8];
		halfmove_clock = 0;
		break;
	}
	case CASTLE:
	{
		moves_stack[moves_stack_size].captured_piece = NONE;
		P[KING][side_to_move] ^= from_to_mask;
		//update zobrist key
		all_pieces ^= from_to_mask;
		//handle the rook
		if (to == 6 || to == 62) //kingside castling
		{
			int rook_from = to + 1;
			int rook_to = to - 1;
			Bitboard rook_from_to_mask = (1ULL << rook_from) | (1ULL << rook_to);
			P[ROOK][side_to_move] ^= rook_from_to_mask;
			all_pieces ^= rook_from_to_mask;
			all_pieces_types[side_to_move] ^= rook_from_to_mask;
			//update zobrist key
			zobrist_key ^= zobrist_pieces[side_to_move][ROOK][rook_from];
			zobrist_key ^= zobrist_pieces[side_to_move][ROOK][rook_to];
		}
		else if (to == 2 || to == 58) //queenside castling
		{
			int rook_from = (to - 2);
			int rook_to = (to + 1);
			Bitboard rook_from_to_mask = (1ULL << rook_from) | (1ULL << rook_to);
			P[ROOK][side_to_move] ^= rook_from_to_mask;
			all_pieces ^= rook_from_to_mask;
			all_pieces_types[side_to_move] ^= rook_from_to_mask;
			//update zobrist key
			zobrist_key ^= zobrist_pieces[side_to_move][ROOK][rook_from];
			zobrist_key ^= zobrist_pieces[side_to_move][ROOK][rook_to];
		}

		if (side_to_move == 0)
		{
			uint8_t castling_rights_old = castling_rights;
			castling_rights &= 0b1100;
			uint8_t castling_change = castling_rights_old ^ castling_rights;
			unsigned long bit_idx;
			while (castling_change)
			{
				_BitScanForward64(&bit_idx, castling_change);
				zobrist_key ^= zobrist_castling[bit_idx];
				castling_change &= castling_change - 1;
			}
		}
		else
		{
			uint8_t castling_rights_old = castling_rights;
			castling_rights &= 0b0011;
			uint8_t castling_change = castling_rights_old ^ castling_rights;
			unsigned long bit_idx;
			while (castling_change)
			{
				_BitScanForward64(&bit_idx, castling_change);
				zobrist_key ^= zobrist_castling[bit_idx];
				castling_change &= castling_change - 1;
			}
		}
		++halfmove_clock;
		break;
	}
	case CAPTURE_WITH_PAWN://no promotion
	{
		PieceType captured_piece;
		//determine captured piece type
		if (P[PAWN][opp] & to_mask)
			captured_piece = PAWN;
		else if (P[KNIGHT][opp] & to_mask)
			captured_piece = KNIGHT;
		else if (P[BISHOP][opp] & to_mask)
			captured_piece = BISHOP;
		else if (P[ROOK][opp] & to_mask)
		{
			captured_piece = ROOK;
			castling_rights &= castling_mask[to];
		}
		else if (P[QUEEN][opp] & to_mask)
			captured_piece = QUEEN;
		else//king cannot be captured therefore this is en passant capture
		{
			moves_stack[moves_stack_size].captured_piece = EMPTY;//EMPTY means en passant capture

			Bitboard en_passant_pawn_mask;
			if (side_to_move == 0)
			{
				en_passant_pawn_mask = to_mask >> 8;
				P[PAWN][1] ^= en_passant_pawn_mask;

				zobrist_key ^= zobrist_pieces[1][PAWN][to - 8];
			}
			else
			{
				en_passant_pawn_mask = to_mask << 8;
				P[PAWN][0] ^= en_passant_pawn_mask;

				zobrist_key ^= zobrist_pieces[0][PAWN][to + 8];
			}
			P[PAWN][side_to_move] ^= from_to_mask;
			all_pieces_types[opp] ^= en_passant_pawn_mask;
			all_pieces ^= en_passant_pawn_mask | from_to_mask;
			//update pawn count on files
			int to_file = to % 8;
			--number_of_pawns_on_files[opp][to_file];
			--number_of_pawns_on_files[side_to_move][from % 8];
			++number_of_pawns_on_files[side_to_move][to_file];
			halfmove_clock = 0;
			break;
		}
		moves_stack[moves_stack_size].captured_piece = captured_piece;
		P[PAWN][side_to_move] ^= from_to_mask;
		P[captured_piece][opp] ^= to_mask;
		all_pieces_types[opp] ^= to_mask;
		all_pieces ^= from_mask;
		//update sobrist key
		zobrist_key ^= zobrist_pieces[opp][captured_piece][to];
		//update pawn count on files
		int to_file = to % 8;
		--number_of_pawns_on_files[side_to_move][from % 8];
		++number_of_pawns_on_files[side_to_move][to_file];
		if (captured_piece == PAWN)
			--number_of_pawns_on_files[opp][to_file];
		break;
	}
	case CAPTURE_WITH_ROOK:
	{
		//update castling rights
		uint8_t castling_rights_old = castling_rights;
		castling_rights &= castling_mask[to];
		uint8_t castling_change = castling_rights_old ^ castling_rights;
		unsigned long bit_idx;
		while (castling_change)
		{
			_BitScanForward64(&bit_idx, castling_change);
			zobrist_key ^= zobrist_castling[bit_idx];
			castling_change &= castling_change - 1;
		}
	}
	case CAPTURE_WITH_KNIGHT:
	case CAPTURE_WITH_BISHOP:
	case CAPTURE_WITH_QUEEN:
	{
		PieceType captured_piece;
		//determine captured piece type
		if (P[PAWN][opp] & to_mask)
			captured_piece = PAWN;
		else if (P[KNIGHT][opp] & to_mask)
			captured_piece = KNIGHT;
		else if (P[BISHOP][opp] & to_mask)
			captured_piece = BISHOP;
		else if (P[ROOK][opp] & to_mask)
			captured_piece = ROOK;
		else if (P[QUEEN][opp] & to_mask)
			captured_piece = QUEEN;
		moves_stack[moves_stack_size].captured_piece = captured_piece;
		P[piece_moved][side_to_move] ^= from_to_mask;
		P[captured_piece][opp] ^= to_mask;
		all_pieces_types[opp] ^= to_mask;
		all_pieces ^= from_mask;
		if (captured_piece == PAWN)
			--number_of_pawns_on_files[opp][to % 8];

		//update zobrist key
		zobrist_key ^= zobrist_pieces[opp][captured_piece][to];


		uint8_t castling_rights_old = castling_rights;
		castling_rights &= castling_mask[to];
		uint8_t castling_change = castling_rights_old ^ castling_rights;
		unsigned long bit_idx;
		while (castling_change)
		{
			_BitScanForward64(&bit_idx, castling_change);
			zobrist_key ^= zobrist_castling[bit_idx];
			castling_change &= castling_change - 1;
		}
		halfmove_clock = 0;
		break;
	}
	case CAPTURE_WITH_KING:
	{
		PieceType captured_piece;
		//determine captured piece type
		if (P[PAWN][opp] & to_mask)
			captured_piece = PAWN;
		else if (P[KNIGHT][opp] & to_mask)
			captured_piece = KNIGHT;
		else if (P[BISHOP][opp] & to_mask)
			captured_piece = BISHOP;
		else if (P[ROOK][opp] & to_mask)
			captured_piece = ROOK;
		else if (P[QUEEN][opp] & to_mask)
			captured_piece = QUEEN;
		moves_stack[moves_stack_size].captured_piece = captured_piece;
		P[KING][side_to_move] ^= from_to_mask;
		P[captured_piece][opp] ^= to_mask;
		all_pieces_types[opp] ^= to_mask;
		all_pieces ^= from_mask;
		if (captured_piece == PAWN)
			--number_of_pawns_on_files[opp][to % 8];
		//update sobrist key
		zobrist_key ^= zobrist_pieces[opp][captured_piece][to];


		uint8_t castling_rights_old = castling_rights;
		castling_rights &= castling_mask[from];
		castling_rights &= castling_mask[to];

		uint8_t castling_change = castling_rights_old ^ castling_rights;
		unsigned long bit_idx;
		while (castling_change)
		{
			_BitScanForward64(&bit_idx, castling_change);
			zobrist_key ^= zobrist_castling[bit_idx];
			castling_change &= castling_change - 1;
		}
		halfmove_clock = 0;
		break;
	}
	case QUIET_ROOK:
	{
		//update castling rights
		uint8_t castling_rights_old = castling_rights;
		castling_rights &= castling_mask[from];
		uint8_t castling_change = castling_rights_old ^ castling_rights;
		unsigned long bit_idx;
		while (castling_change)
		{
			_BitScanForward64(&bit_idx, castling_change);
			zobrist_key ^= zobrist_castling[bit_idx];
			castling_change &= castling_change - 1;
		}
	}
	case QUIET_KNIGHT:
	case QUIET_BISHOP:
	case QUIET_QUEEN:
	{
		moves_stack[moves_stack_size].captured_piece = NONE;
		P[piece_moved][side_to_move] ^= from_to_mask;
		all_pieces ^= from_to_mask;
		++halfmove_clock;
		break;
	}
	case QUIET_PAWN:
	{
		moves_stack[moves_stack_size].captured_piece = NONE;
		P[PAWN][side_to_move] ^= from_to_mask;
		all_pieces ^= from_to_mask;
		halfmove_clock = 0;
		if ((from - to == 16) || (to - from == 16))//double push
		{
			en_passant_square = (from + to) >> 1;
		}
		break;
	}
	case QUIET_KING:
	{
		moves_stack[moves_stack_size].captured_piece = NONE;
		P[KING][side_to_move] ^= from_to_mask;
		all_pieces ^= from_to_mask;


		uint8_t castling_rights_old = castling_rights;
		castling_rights &= castling_mask[from];
		castling_rights &= castling_mask[to];

		uint8_t castling_change = castling_rights_old ^ castling_rights;
		unsigned long bit_idx;
		while (castling_change)
		{
			_BitScanForward64(&bit_idx, castling_change);
			zobrist_key ^= zobrist_castling[bit_idx];
			castling_change &= castling_change - 1;
		}
		++halfmove_clock;

		break;
	}
	}
	zobrist_key ^= zobrist_en_passant[en_passant_square];

	++moves_stack_size;

	side_to_move = opp;
	zobrist_key ^= zobrist_side_to_move;


}

void Board::undo_move()
{
	zobrist_key ^= zobrist_en_passant[en_passant_square];
	en_passant_square = moves_stack[--moves_stack_size].en_passant_square;
	uint8_t castling_rights_old = castling_rights;
	castling_rights = moves_stack[moves_stack_size].castling_rights;
	halfmove_clock = moves_stack[moves_stack_size].halfmove_clock;

	int opp = side_to_move ^ 1;//opp is the side that made the move to be undone

	MoveType move_type = moves_stack[moves_stack_size].move_type;
	uint16_t move = moves_stack[moves_stack_size].move;
	PieceType captured_piece = moves_stack[moves_stack_size].captured_piece;

	Bitboard to_mask = mg.to_mask[move];
	Bitboard from_mask = mg.from_mask[move];
	Bitboard from_to_mask = mg.from_to_mask[move];

	uint8_t from = move & 0b111111;
	uint8_t to = move >> 6;
	PieceType piece_moved = static_cast<PieceType>(move_type & 0b111);

	//zobrist key update
	zobrist_key ^= zobrist_pieces[opp][piece_moved][to];
	zobrist_key ^= zobrist_pieces[opp][piece_moved][from];
	zobrist_key ^= zobrist_en_passant[en_passant_square];

	uint8_t castling_change = castling_rights_old ^ castling_rights;
	unsigned long bit_idx;
	while (castling_change)
	{
		_BitScanForward64(&bit_idx, castling_change);
		zobrist_key ^= zobrist_castling[bit_idx];
		castling_change &= castling_change - 1;
	}

	all_pieces_types[opp] ^= from_to_mask;
	switch (move_type)
	{
	case QUEEN_PROMOTION:
	{
		P[PAWN][opp] |= from_mask;
		P[QUEEN][opp] ^= to_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[opp][QUEEN][to];
		zobrist_key ^= zobrist_pieces[opp][PAWN][to];
		if (captured_piece != NONE)
		{
			P[captured_piece][side_to_move] |= to_mask;
			all_pieces_types[side_to_move] ^= to_mask;
			all_pieces ^= from_mask;
			zobrist_key ^= zobrist_pieces[side_to_move][captured_piece][to];
			++number_of_pawns_on_files[opp][from % 8];
			break;
		}
		all_pieces ^= from_to_mask;
		++number_of_pawns_on_files[opp][from % 8];
		break;
	}
	case KNIGHT_PROMOTION:
	{
		P[PAWN][opp] |= from_mask;
		P[KNIGHT][opp] ^= to_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[opp][KNIGHT][to];
		zobrist_key ^= zobrist_pieces[opp][PAWN][to];
		if (captured_piece != NONE)
		{
			P[captured_piece][side_to_move] |= to_mask;
			all_pieces_types[side_to_move] ^= to_mask;
			all_pieces ^= from_mask;
			zobrist_key ^= zobrist_pieces[side_to_move][captured_piece][to];
			++number_of_pawns_on_files[opp][from % 8];
			break;
		}
		all_pieces ^= from_to_mask;
		++number_of_pawns_on_files[opp][from % 8];
		break;
	}
	case ROOK_PROMOTION:
	{
		P[PAWN][opp] |= from_mask;
		P[ROOK][opp] ^= to_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[opp][ROOK][to];
		zobrist_key ^= zobrist_pieces[opp][PAWN][to];
		if (captured_piece != NONE)
		{
			P[captured_piece][side_to_move] |= to_mask;
			all_pieces_types[side_to_move] ^= to_mask;
			all_pieces ^= from_mask;
			zobrist_key ^= zobrist_pieces[side_to_move][captured_piece][to];
			++number_of_pawns_on_files[opp][from % 8];
			break;
		}
		all_pieces ^= from_to_mask;
		++number_of_pawns_on_files[opp][from % 8];
		break;
	}
	case BISHOP_PROMOTION:
	{
		P[PAWN][opp] |= from_mask;
		P[BISHOP][opp] ^= to_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[opp][BISHOP][to];
		zobrist_key ^= zobrist_pieces[opp][PAWN][to];
		if (captured_piece != NONE)
		{
			P[captured_piece][side_to_move] |= to_mask;
			all_pieces_types[side_to_move] ^= to_mask;
			all_pieces ^= from_mask;
			zobrist_key ^= zobrist_pieces[side_to_move][captured_piece][to];
			++number_of_pawns_on_files[opp][from % 8];
			break;
		}
		all_pieces ^= from_to_mask;
		++number_of_pawns_on_files[opp][from % 8];
		break;
	}
	case CASTLE:
	{
		P[KING][opp] ^= from_to_mask;
		all_pieces ^= from_to_mask;
		//handle the rook
		if (to == 6 || to == 62) //kingside castling
		{
			uint8_t rook_from_square = to + 1;
			uint8_t rook_to_square = to - 1;
			Bitboard rook_from_to_mask = (1ULL << rook_to_square) | (1ULL << rook_from_square);
			P[ROOK][opp] ^= rook_from_to_mask;
			zobrist_key ^= zobrist_pieces[opp][ROOK][rook_from_square];
			zobrist_key ^= zobrist_pieces[opp][ROOK][rook_to_square];
			all_pieces ^= rook_from_to_mask;
			all_pieces_types[opp] ^= rook_from_to_mask;
		}
		else if (to == 2 || to == 58) //queenside castling
		{
			uint8_t rook_from_square = to - 2;
			uint8_t rook_to_square = to + 1;
			Bitboard rook_from_to_mask = (1ULL << rook_to_square) | (1ULL << rook_from_square);
			P[ROOK][opp] ^= rook_from_to_mask;
			zobrist_key ^= zobrist_pieces[opp][ROOK][rook_from_square];
			zobrist_key ^= zobrist_pieces[opp][ROOK][rook_to_square];
			all_pieces ^= rook_from_to_mask;
			all_pieces_types[opp] ^= rook_from_to_mask;
		}
		break;
	}
	case CAPTURE_WITH_PAWN:
	{
		if (captured_piece == EMPTY)
		{
			Bitboard en_passant_pawn_mask;
			if (side_to_move == 0)
			{
				en_passant_pawn_mask = to_mask << 8;
				zobrist_key ^= zobrist_pieces[0][PAWN][to + 8];
			}
			else
			{
				en_passant_pawn_mask = to_mask >> 8;
				zobrist_key ^= zobrist_pieces[1][PAWN][to - 8];
			}
			P[PAWN][side_to_move] |= en_passant_pawn_mask;
			P[PAWN][opp] ^= from_to_mask;
			all_pieces_types[side_to_move] ^= en_passant_pawn_mask;
			all_pieces ^= en_passant_pawn_mask | from_to_mask;
			//update pawn count on files
			int to_file = to % 8;
			++number_of_pawns_on_files[side_to_move][to_file];
			++number_of_pawns_on_files[opp][from % 8];
			--number_of_pawns_on_files[opp][to_file];
			break;
		}
		P[PAWN][opp] ^= from_to_mask;
		P[captured_piece][side_to_move] ^= to_mask;
		all_pieces_types[side_to_move] ^= to_mask;
		all_pieces ^= from_mask;
		//update sobrist key
		zobrist_key ^= zobrist_pieces[side_to_move][captured_piece][to];
		//update pawn count on files
		int from_file = (from) % 8;
		int to_file = (to) % 8;
		++number_of_pawns_on_files[opp][from_file];
		--number_of_pawns_on_files[opp][to_file];
		if (captured_piece == PAWN)
			++number_of_pawns_on_files[side_to_move][to_file];
		break;
	}
	case CAPTURE_WITH_KNIGHT:
	case CAPTURE_WITH_BISHOP:
	case CAPTURE_WITH_ROOK:
	case CAPTURE_WITH_QUEEN:
	case CAPTURE_WITH_KING:
	{
		P[piece_moved][opp] ^= from_to_mask;
		P[captured_piece][side_to_move] ^= to_mask;
		all_pieces_types[side_to_move] ^= to_mask;
		all_pieces ^= from_mask;
		//update zobrist key
		zobrist_key ^= zobrist_pieces[side_to_move][captured_piece][to];
		//update pawn count on files
		if (captured_piece == PAWN)
			++number_of_pawns_on_files[side_to_move][to % 8];
		break;
	}
	case QUIET_KNIGHT:
	case QUIET_BISHOP:
	case QUIET_ROOK:
	case QUIET_QUEEN:
	case QUIET_KING:
	case QUIET_PAWN:
	{
		P[piece_moved][opp] ^= from_to_mask;
		all_pieces ^= from_to_mask;
		break;
	}
	}



	
	side_to_move ^= 1;
	zobrist_key ^= zobrist_side_to_move;
}


void Board::update_all_pieces_bitboards()
{
	all_pieces_types[0] = P[PAWN][0] | P[KNIGHT][0] | P[BISHOP][0] | P[ROOK][0] | P[QUEEN][0] | P[KING][0];
	all_pieces_types[1] = P[PAWN][1] | P[KNIGHT][1] | P[BISHOP][1] | P[ROOK][1] | P[QUEEN][1] | P[KING][1];
	all_pieces = all_pieces_types[0] | all_pieces_types[1];
}


std::string move_to_string(SimpleMove move)
{
	std::string files = "abcdefgh";
	std::string ranks = "12345678";
	std::string move_str = "";
	move_str += files[(move & 0b111111) % 8];
	move_str += ranks[(move & 0b111111) / 8];
	move_str += files[(move >> 6) % 8];
	move_str += ranks[(move >> 6) / 8];
	return move_str;
}


constexpr uint16_t index_max_value = 0xFF;

void Board::perft(int depth)
{
	/*
	legal moves are stored in a following order:
	pawn quiet
	pawn capture
	pawn promotions
	knight capture
	knight quiet
	bishop capture
	bishop quiet
	rook capture
	rook quiet
	queen capture
	queen quiet
	king capture
	king quiet
	castling
	*/

	if (depth == 0)
	{
		++perft_nodes_searched;
		return;
	}
	uint64_t nodes = 0ULL;
	mg.generate_pseudo_legal_moves_with_category_ordering();
	mg.filter_pseudo_legal_moves();
	SimpleMove legal_moves[MoveGenerator::max_legal_moves_count];
	MovesIndexes8bit indexes_copy = mg.legal_moves_indexes;
	std::memcpy(legal_moves, mg.legal_moves, ((indexes_copy.castle + 1) & index_max_value) * sizeof(SimpleMove));
	int i = 0;
	for (; i < ((indexes_copy.quiet_pawn + 1) & index_max_value); i++)
	{
		make_move(legal_moves[i], QUIET_PAWN);
		/*if (flag3)
			std::cout << perft_nodes_searched << " ";

		if (!flag1)
			flag1 = perft_nodes_searched == 6692;
		if (!flag2)
			flag2 = perft_nodes_searched == 364 && flag1;
		if (!flag3)
			flag3 = perft_nodes_searched == 30 && flag2;
		else
		{
			std::cout << "here";
		}*/
		//if (depth == test_depth)
		//{
		//std::cout << depth;

		
		


			//std::cout << perft_nodes_searched << " ";
		//}
		perft(depth - 1);
		undo_move();
	}
	for (; i < ((indexes_copy.pawn_capture + 1) & index_max_value); i++)
	{
		make_move(legal_moves[i], CAPTURE_WITH_PAWN);
		perft(depth - 1);
		undo_move();
	}
	for (; i < ((indexes_copy.queen_promotion + 1) & index_max_value); i++)
	{
		make_move(legal_moves[i], QUEEN_PROMOTION);
		perft(depth - 1);
		undo_move();
		make_move(legal_moves[i], KNIGHT_PROMOTION);
		perft(depth - 1);
		undo_move();
		make_move(legal_moves[i], ROOK_PROMOTION);
		perft(depth - 1);
		undo_move();
		make_move(legal_moves[i], BISHOP_PROMOTION);
		perft(depth - 1);
		undo_move();
	}

	for (; i < ((indexes_copy.knight_capture + 1) & index_max_value); i++)
	{
		make_move(legal_moves[i], CAPTURE_WITH_KNIGHT);
		perft(depth - 1);
		undo_move();
	}
	for (; i < ((indexes_copy.quiet_knight + 1) & index_max_value); i++)
	{
		make_move(legal_moves[i], QUIET_KNIGHT);
		perft(depth - 1);
		undo_move();
	}
	for (; i < ((indexes_copy.bishop_capture + 1) & index_max_value); i++)
	{
		make_move(legal_moves[i], CAPTURE_WITH_BISHOP);
		perft(depth - 1);
		undo_move();
	}
	for (; i < ((indexes_copy.quiet_bishop + 1) & index_max_value); i++)
	{
		make_move(legal_moves[i], QUIET_BISHOP);
		perft(depth - 1);
		undo_move();
	}
	for (; i < ((indexes_copy.rook_capture + 1) & index_max_value); i++)
	{
		make_move(legal_moves[i], CAPTURE_WITH_ROOK);
		perft(depth - 1);
		undo_move();
	}
	for (; i < ((indexes_copy.quiet_rook + 1) & index_max_value); i++)
	{
		make_move(legal_moves[i], QUIET_ROOK);
		perft(depth - 1);
		undo_move();
	}
	for (; i < ((indexes_copy.queen_capture + 1) &index_max_value); i++)
	{
		make_move(legal_moves[i], CAPTURE_WITH_QUEEN);
		perft(depth - 1);
		undo_move();
	}
	for (; i< ((indexes_copy.quiet_queen + 1) &index_max_value); i++)
	{
		make_move(legal_moves[i], QUIET_QUEEN);
		perft(depth - 1);
		undo_move();
	}
	for (; i < ((indexes_copy.king_capture + 1) &index_max_value); i++)
	{
		make_move(legal_moves[i], CAPTURE_WITH_KING);
		perft(depth - 1);
		undo_move();
	}
	for (; i< ((indexes_copy.quiet_king + 1) &index_max_value); i++)
	{
		make_move(legal_moves[i], QUIET_KING);
		perft(depth - 1);
		undo_move();
	}
	for (; i< ((indexes_copy.castle + 1) &index_max_value); i++)
	{
		make_move(legal_moves[i], CASTLE);
		perft(depth - 1);
		undo_move();
	}
}



void Board::initial_perft(int depth)
{
	perft_nodes_searched = 0;
	/*
	legal moves are stored in a following order:
	pawn quiet
	pawn capture
	pawn promotions
	knight capture
	knight quiet
	bishop capture
	bishop quiet
	rook capture
	rook quiet
	queen capture
	queen quiet
	king capture
	king quiet
	castling
	*/

	if (depth == 0)
	{
		++perft_nodes_searched;
		return;
	}

	mg.generate_pseudo_legal_moves_with_category_ordering();
	mg.filter_pseudo_legal_moves();
	SimpleMove legal_moves[MoveGenerator::max_legal_moves_count];
	MovesIndexes8bit indexes_copy = mg.legal_moves_indexes;
	std::memcpy(legal_moves, mg.legal_moves, ((indexes_copy.castle + 1) & index_max_value) * sizeof(SimpleMove));
	int i = 0;
	uint64_t nodes_prev = 0;
	for (; i < ((indexes_copy.quiet_pawn + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], QUIET_PAWN);
		perft(depth - 1);
		undo_move();
		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;

	}
	for (; i < ((indexes_copy.pawn_capture + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], CAPTURE_WITH_PAWN);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
	for (; i < ((indexes_copy.queen_promotion + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], QUEEN_PROMOTION);
		perft(depth - 1);
		undo_move();
		make_move(legal_moves[i], KNIGHT_PROMOTION);
		perft(depth - 1);
		undo_move();
		make_move(legal_moves[i], ROOK_PROMOTION);
		perft(depth - 1);
		undo_move();
		make_move(legal_moves[i], BISHOP_PROMOTION);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}

	for (; i < ((indexes_copy.knight_capture + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], CAPTURE_WITH_KNIGHT);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
	for (; i < ((indexes_copy.quiet_knight + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], QUIET_KNIGHT);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
	for (; i < ((indexes_copy.bishop_capture + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], CAPTURE_WITH_BISHOP);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
	for (; i < ((indexes_copy.quiet_bishop + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], QUIET_BISHOP);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
	for (; i < ((indexes_copy.rook_capture + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], CAPTURE_WITH_ROOK);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
	for (; i < ((indexes_copy.quiet_rook + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], QUIET_ROOK);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
	for (; i < ((indexes_copy.queen_capture + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], CAPTURE_WITH_QUEEN);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
	for (; i < ((indexes_copy.quiet_queen + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], QUIET_QUEEN);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
	for (; i < ((indexes_copy.king_capture + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], CAPTURE_WITH_KING);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
	for (; i < ((indexes_copy.quiet_king + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], QUIET_KING);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
	for (; i < ((indexes_copy.castle + 1) & index_max_value); i++)
	{
		nodes_prev = perft_nodes_searched;
		make_move(legal_moves[i], CASTLE);
		perft(depth - 1);
		undo_move();

		uint64_t nodes_for_move = perft_nodes_searched - nodes_prev;
		std::cout << move_to_string(legal_moves[i]) << ": " << nodes_for_move << std::endl;
	}
}


void Board::display_board(std::ostream& output)
{
	//debug only
	output << "Side to move: " << (side_to_move == 0 ? "White" : "Black") << "\n";
	output << "En passant square: " << en_passant_square << "\n";
	output << "Halfmove clock: " << halfmove_clock << "\n";

	for (int rank = 7; rank >= 0; rank--)
	{
		output << rank + 1 << " "; // rank on the left
		for (int file = 0; file < 8; file++)
		{
			int square = rank * 8 + file;
			Bitboard mask = 1ULL << square;
			char piece_char = '.';
			if (P[PAWN][0] & mask) piece_char = 'P';
			else if (P[KNIGHT][0] & mask) piece_char = 'N';
			else if (P[BISHOP][0] & mask) piece_char = 'B';
			else if (P[ROOK][0] & mask) piece_char = 'R';
			else if (P[QUEEN][0] & mask) piece_char = 'Q';
			else if (P[KING][0] & mask) piece_char = 'K';
			else if (P[PAWN][1] & mask) piece_char = 'p';
			else if (P[KNIGHT][1] & mask) piece_char = 'n';
			else if (P[BISHOP][1] & mask) piece_char = 'b';
			else if (P[ROOK][1] & mask) piece_char = 'r';
			else if (P[QUEEN][1] & mask) piece_char = 'q';
			else if (P[KING][1] & mask) piece_char = 'k';
			output << piece_char << " ";
		}
		output << "\n";
	}

	// print file letters at bottom
	output << "  a b c d e f g h" << "\n";
	output << "\n";
	output << std::flush;
}

void Board::display_board()
{
	//debug only
	std::cout << "Side to move: " << (side_to_move == 0 ? "White" : "Black") << "\n";
	std::cout << "En passant square: " << en_passant_square << "\n";
	std::cout << "Halfmove clock: " << halfmove_clock << "\n";

	for (int rank = 7; rank >= 0; rank--)
	{
		std::cout << rank + 1 << " "; // rank on the left
		for (int file = 0; file < 8; file++)
		{
			int square = rank * 8 + file;
			Bitboard mask = 1ULL << square;
			char piece_char = '.';
			if (P[PAWN][0] & mask) piece_char = 'P';
			else if (P[KNIGHT][0] & mask) piece_char = 'N';
			else if (P[BISHOP][0] & mask) piece_char = 'B';
			else if (P[ROOK][0] & mask) piece_char = 'R';
			else if (P[QUEEN][0] & mask) piece_char = 'Q';
			else if (P[KING][0] & mask) piece_char = 'K';
			else if (P[PAWN][1] & mask) piece_char = 'p';
			else if (P[KNIGHT][1] & mask) piece_char = 'n';
			else if (P[BISHOP][1] & mask) piece_char = 'b';
			else if (P[ROOK][1] & mask) piece_char = 'r';
			else if (P[QUEEN][1] & mask) piece_char = 'q';
			else if (P[KING][1] & mask) piece_char = 'k';
			std::cout << piece_char << " ";
		}
		std::cout << "\n";
	}

	// print file letters at bottom
	std::cout << "  a b c d e f g h" << "\n";
	std::cout << "\n";
	std::cout << std::flush;
}


