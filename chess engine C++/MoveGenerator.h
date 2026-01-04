#pragma once
#include <vector>
#include "Move.h"
#include "MoveType.h"
#include "MovesIndexes8bit.h"
#include "MovesIndexes16bit.h"

typedef uint64_t Bitboard;
typedef uint16_t SimpleMove;


class Board;

class MoveGenerator
{
private:
	Board* const board;

	constexpr static int max_pseudo_legal_moves_count = 512;
	SimpleMove pseudo_legal_moves[max_pseudo_legal_moves_count];//save psudo legal moves limit
	int pseudo_legal_moves_last_idx;

	MovesIndexes16bit pseudo_legal_moves_indexes = MovesIndexes16bit();


	std::vector<Move> legal_moves_vector;//only for get_legal_moves return
	int legal_moves_vector_count = 0;

	std::vector<Move> pseudo_legal_moves_vector;//only for get_pseudo_legal_moves return
	int pseudo_legal_moves_vector_count = 0;





	





	void generate_relevant_blockers();
	void generate_attack_tables();
	void generate_from_to_masks();




public:
	//masks
	constexpr static Bitboard FILE_A = 0x0101010101010101ULL;
	constexpr static Bitboard FILE_B = 0x0202020202020202ULL;
	constexpr static Bitboard FILE_C = 0x0404040404040404ULL;
	constexpr static Bitboard FILE_D = 0x0808080808080808ULL;
	constexpr static Bitboard FILE_E = 0x1010101010101010ULL;
	constexpr static Bitboard FILE_F = 0x2020202020202020ULL;
	constexpr static Bitboard FILE_G = 0x4040404040404040ULL;
	constexpr static Bitboard FILE_H = 0x8080808080808080ULL;

	constexpr static Bitboard RANK_1 = 0x00000000000000FFULL;
	constexpr static Bitboard RANK_2 = 0x000000000000FF00ULL;
	constexpr static Bitboard RANK_3 = 0x0000000000FF0000ULL;
	constexpr static Bitboard RANK_4 = 0x00000000FF000000ULL;
	constexpr static Bitboard RANK_5 = 0x000000FF00000000ULL;
	constexpr static Bitboard RANK_6 = 0x0000FF0000000000ULL;
	constexpr static Bitboard RANK_7 = 0x00FF000000000000ULL;
	constexpr static Bitboard RANK_8 = 0xFF00000000000000ULL;

	constexpr static Bitboard FILE_A_NEGATION = ~FILE_A;
	constexpr static Bitboard FILE_B_NEGATION = ~FILE_B;
	constexpr static Bitboard FILE_C_NEGATION = ~FILE_C;
	constexpr static Bitboard FILE_D_NEGATION = ~FILE_D;
	constexpr static Bitboard FILE_E_NEGATION = ~FILE_E;
	constexpr static Bitboard FILE_F_NEGATION = ~FILE_F;
	constexpr static Bitboard FILE_G_NEGATION = ~FILE_G;
	constexpr static Bitboard FILE_H_NEGATION = ~FILE_H;

	constexpr static Bitboard RANK_1_NEGATION = ~RANK_1;
	constexpr static Bitboard RANK_2_NEGATION = ~RANK_2;
	constexpr static Bitboard RANK_3_NEGATION = ~RANK_3;
	constexpr static Bitboard RANK_4_NEGATION = ~RANK_4;
	constexpr static Bitboard RANK_5_NEGATION = ~RANK_5;
	constexpr static Bitboard RANK_6_NEGATION = ~RANK_6;
	constexpr static Bitboard RANK_7_NEGATION = ~RANK_7;
	constexpr static Bitboard RANK_8_NEGATION = ~RANK_8;

	//masks for promotins with captures
	constexpr static Bitboard RANK_2_WITHOUT_A_FILE = RANK_2 & FILE_A_NEGATION;//black left capture
	constexpr static Bitboard RANK_7_WITHOUT_A_FILE = RANK_7 & FILE_H_NEGATION;//white left capture
	constexpr static Bitboard RANK_2_WITHOUT_H_FILE = RANK_2 & FILE_H_NEGATION;//black right capture
	constexpr static Bitboard RANK_7_WITHOUT_H_FILE = RANK_7 & FILE_A_NEGATION;//white right capture

	constexpr static Bitboard WHITE_KINGSIDE_CASTLE_MASK = 0b11ULL << 5;
	constexpr static Bitboard WHITE_QUEENSIDE_CASTLE_MASK = 0b111ULL << 1;
	constexpr static Bitboard BLACK_KINGSIDE_CASTLE_MASK = 0b11ULL << 61;
	constexpr static Bitboard BLACK_QUEENSIDE_CASTLE_MASK = 0b111ULL << 57;

	constexpr static uint16_t WHITE_KINGSIDE_CASTLE_FROM_TO_MASK = 4 | (6 << 6);
	constexpr static uint16_t WHITE_QUEENSIDE_CASTLE_FROM_TO_MASK = 4 | (2 << 6);
	constexpr static uint16_t BLACK_KINGSIDE_CASTLE_FROM_TO_MASK = 60 | (62 << 6);
	constexpr static uint16_t BLACK_QUEENSIDE_CASTLE_FROM_TO_MASK = 60 | (58 << 6);





	Bitboard bishop_relevant_blockers[64];
	Bitboard rook_relevant_blockers[64];
	Bitboard bishop_attack_tables[64][512];//max 512 possible attacks for a bishop on any square
	Bitboard rook_attack_tables[64][4096];//max 4096 possible attacks for a rook on any square
	Bitboard knight_attack_tables[64];
	Bitboard king_attack_tables[64];
	Bitboard pawn_attack_tables[2][64];

	//masks of pieces which are the actual blockers for each square
	Bitboard bishop_blockers[64][512];
	Bitboard rook_blockers[64][4096];

	uint8_t bishop_relevant_bits_shift[64];
	uint8_t rook_relevant_bits_shift[64];

	Bitboard bishop_magic_numbers[64] = {
	0x20060440540040,
	0x30210830848002,
	0x4104402a4200310,
	0x620a0202000020,
	0x144142003002800,
	0x118620e0000080,
	0x40840420260008,
	0x220110085200,
	0x25056084140085,
	0x3200403114104,
	0x185408420c022000,
	0x108040412802300,
	0x80400404a0002100,
	0x4100420804c40104,
	0x800c10430041600,
	0x4400010108012400,
	0xb018012020812200,
	0x4041001881510,
	0x40880c0410640010,
	0x88801806004000,
	0x404000a20a00008,
	0xa08010c0602002,
	0x4181110448021004,
	0x2040440022080400,
	0x1420800508d010,
	0x2002218010290200,
	0x128080801020020,
	0x90248000a820040,
	0x3002840100802000,
	0x900410002010901,
	0x4401204004040440,
	0x2005206f2410406,
	0xa40a031028612020,
	0x40c4200049042,
	0x10810808018102c0,
	0x408020080180380,
	0x410020202022008,
	0x200c0100182081,
	0x200a008100060802,
	0x24084200018182,
	0x204104809108420,
	0x1010806406080,
	0x440140254228800,
	0x1000002128040401,
	0x7000080104000040,
	0x10b120587000200,
	0x248068404080140,
	0x1202081201940820,
	0x821012120220400,
	0x8080828821108010,
	0x500020082212402,
	0x81980c504881001,
	0x40084122821201,
	0xa1059120120,
	0x8220200401604208,
	0x10810010d450020,
	0x22840441101800,
	0x2140508218450400,
	0x1a0810084218810,
	0x1002000000209800,
	0x4120120481,
	0x880820180480,
	0x8108220650010100,
	0x20040888010020
	};
	Bitboard rook_magic_numbers[64] = {
		0x48000104000a480,
		0x340004070002008,
		0x80100068802000,
		0x100282010000d00,
		0x95001801000c0a10,
		0x100081400120300,
		0x200010200218408,
		0x50000208a005100,
		0x20028000a0804004,
		0x5004008250080,
		0x1202001382420020,
		0x1009001000482104,
		0x804400480180,
		0x8010808074000a00,
		0x1a000401180200,
		0x181001100104096,
		0xca4088800140002c,
		0x8010094020004000,
		0x800808020011004,
		0x1042020008411020,
		0x802a50011000800,
		0x80808004000200,
		0x1c00040042100851,
		0x21100a0004008261,
		0x8000c040002008,
		0x220810700400020,
		0x4441100080200083,
		0x2100100482101,
		0x9c008080280055,
		0x100200801403004,
		0x400184000a4810,
		0x200440a000091c9,
		0x824000800028,
		0x408c81c000802000,
		0x42208b082002040,
		0x1800210009001001,
		0x1002800800800400,
		0x90200800e800400,
		0x818221014000108,
		0x608111840a000041,
		0x400120818000,
		0x9040012810002000,
		0x804200520020,
		0x4082004008220010,
		0x120080004008080,
		0x1000844010002,
		0x4080130850040022,
		0x41009042000c,
		0x501a64008801080,
		0x480200040008980,
		0x81004038200100,
		0x408100348028080,
		0x8000800800440080,
		0x8020800a01440080,
		0x8043008090a0400,
		0x5000104401008200,
		0x8300410410628001,
		0x2440801200210042,
		0x10ed600009004091,
		0x42010010901,
		0x282201080420100a,
		0x402000104081002,
		0x81000800ca100b14,
		0x60008b80c5040022
	};


	//knights
	Bitboard white_kingside_castle_knight_attack_mask;
	Bitboard white_queenside_castle_knight_attack_mask;
	Bitboard black_kingside_castle_knight_attack_mask;
	Bitboard black_queenside_castle_knight_attack_mask;

	//pawns
	Bitboard white_kingside_castle_pawn_attack_mask;
	Bitboard white_queenside_castle_pawn_attack_mask;
	Bitboard black_kingside_castle_pawn_attack_mask;
	Bitboard black_queenside_castle_pawn_attack_mask;

	//king
	Bitboard white_kingside_castle_king_attack_mask;
	Bitboard white_queenside_castle_king_attack_mask;
	Bitboard black_kingside_castle_king_attack_mask;
	Bitboard black_queenside_castle_king_attack_mask;


	Bitboard from_to_mask[4096];
	Bitboard from_mask[4096];
	Bitboard to_mask[4096];
	Bitboard to_negation_mask[4096];


	uint8_t all_attacks_count[2][64];



	static constexpr uint8_t max_legal_moves_count = 255;
	SimpleMove legal_moves[MoveGenerator::max_legal_moves_count];//218 is a theoritiacl limit
	int legal_moves_last_idx = 0;

	MovesIndexes8bit legal_moves_indexes = MovesIndexes8bit();

	bool knight_check;
	bool pawn_check;
	bool bishop_check;
	bool rook_check;
	bool queen_check;
	bool in_check;



	MoveGenerator(Board* board);
	PieceType get_piece_type(const Bitboard* P, unsigned int to_square, unsigned int side);
	void generate_pseudo_legal_attacks(uint8_t side_to_move);//only fills all_attacks_count
	void generate_pseudo_legal_moves_with_category_ordering();
	void filter_pseudo_legal_moves();
	std::vector<Move> get_legal_moves();
	std::vector<Move> get_pseudo_legal_moves();
	bool can_castle(uint8_t castle_type);//0 - white kingside, 1 - white queenside, 2 - black kingside, 3 - black queenside
};

