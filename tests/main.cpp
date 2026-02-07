#include <iostream>
#include "../chess engine C++/Board.h"
#include "../chess engine C++/Engine.h"
#include "../chess engine C++/Move.h"
#include "unit_tests.h"
#include "performance_tests.h"
/*#include "../chess engine C++/Board.h"
#include "../chess engine C++/Move.h"
#include "../chess engine C++/Move.cpp"
#include "../chess engine C++/Board.cpp"
#include "../chess engine C++/Engine.h"
#include "../chess engine C++/Engine.cpp"*/
#include <string>
#include <chrono>


std::string move_to_string2(SimpleMove move)
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

SimpleMove create_simple_move(std::string move)
{
	std::string files = "abcdefgh";
	std::string ranks = "12345678";
	int from_file = files.find(move[0]);
	int from_rank = ranks.find(move[1]);
	int to_file = files.find(move[2]);
	int to_rank = ranks.find(move[3]);
	int from_square = from_rank * 8 + from_file;
	int to_square = to_rank * 8 + to_file;
	return (to_square << 6) | from_square;
}

std::string chess_notation2(uint16_t move)//convert move to chess notation for easier debugging
{
	const std::string files = "abcdefgh";
	const std::string ranks = "12345678";
	uint8_t from_square = move & 0x3F; // Extract the first 6 bits
	uint8_t to_square = (move >> 6); // Extract the next 6 bits
	if (move >> 12)//is required to be zero
		throw std::runtime_error("Critical error: Move variable has first 4 bits non-zero\n");
	std::string notation;
	notation += files[from_square % 8];
	notation += ranks[from_square / 8];
	notation += files[to_square % 8];
	notation += ranks[to_square / 8];
	return notation;
}

PieceType get_piece_on_square(Board* board, uint8_t square)
{
	Bitboard mask = 1ULL << square;
	for (int piece = PAWN; piece <= KING; ++piece)
	{
		if (board->P[piece][0] & mask)
			return static_cast<PieceType>(piece);
		if (board->P[piece][1] & mask)
			return static_cast<PieceType>(piece);
	}
	return EMPTY;
}

PieceType get_piece_on_square_debug(Board* board, uint8_t square)
{
	Bitboard mask = 1ULL << square;
	for (int piece = PAWN; piece <= KING; ++piece)
	{
		if (board->P[piece][0] & mask)
			return static_cast<PieceType>(piece);
		if (board->P[piece][1] & mask)
			return static_cast<PieceType>(piece);
	}
	board->display_board();
	return EMPTY;
}


MoveType get_move_type_from_squares(Board* board, uint8_t from_square, uint8_t to_square)
{
	PieceType piece = get_piece_on_square_debug(board, from_square);
	PieceType target_piece = get_piece_on_square(board, to_square);
	if (piece == PAWN)
	{
		if (to_square == from_square + 8 || to_square == from_square - 8 || to_square == from_square + 16 || to_square == from_square - 16)//quiet move
		{
			if (to_square >= 56 || to_square < 8)//promotion
				return QUEEN_PROMOTION;
			else
				return QUIET_PAWN;
		}
		else//capture
		{
			if (to_square >= 56 || to_square < 8)//promotion with capture
				return QUEEN_PROMOTION;
			else
				return CAPTURE_WITH_PAWN;
		}
	}
	else if (piece == KNIGHT)
	{
		if (target_piece != EMPTY)
			return CAPTURE_WITH_KNIGHT;
		else
			return QUIET_KNIGHT;
	}
	else if (piece == BISHOP)
	{
		if (target_piece != EMPTY)
			return CAPTURE_WITH_BISHOP;
		else
			return QUIET_BISHOP;
	}
	else if (piece == ROOK)
	{
		if (target_piece != EMPTY)
			return CAPTURE_WITH_ROOK;
		else
			return QUIET_ROOK;
	}
	else if (piece == QUEEN)
	{
		if (target_piece != EMPTY)
			return CAPTURE_WITH_QUEEN;
		else
			return QUIET_QUEEN;
	}
	else if (piece == KING)
	{
		if (abs(int(to_square) - int(from_square)) == 2)//castle
			return CASTLE;
		if (target_piece != EMPTY)
			return CAPTURE_WITH_KING;
		else
			return QUIET_KING;
	}
	throw std::runtime_error("error: get_move_type_from_squares failed to determine move type\n");
}


void self_play(int n, uint8_t depth, bool advance_print=false, float time_per_move=0, std::string fen="")
{
	Board board;
	board.initialize_board();
	
	Engine engine(&board);


	if (fen != "")
		board.load_fen(fen);


	board.display_board();
	std::cout << depth << std::endl;
	SearchResult result;
	for (int i = 0; i < n; ++i)
	{
		double total_time = 0;
		auto start = std::chrono::high_resolution_clock::now();
		int j = 0;
		for (;time_per_move!=0 || j<=depth; ++j)
		{
			start = std::chrono::high_resolution_clock::now();
			result = engine.minimax_init(j);
			std::cout << "\rdepth: " << j << " completed";
			std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start;
			total_time += elapsed.count();
			if (time_per_move!=0 && total_time > time_per_move)
				break;
		}
		std::cout << std::endl;

		//debug only
		if (result.best_move.move >= 4096)
		{
			std::cout << "error: best move has invalid move value: " << result.best_move.move << std::endl;
			if (advance_print)
				board.display_board_each_piece_and_side_separately();
			board.display_board();
			std::abort();
		}
		//
		if (result.best_move == Move())
		{
			std::cout << "the game ended" << std::endl;
			if (advance_print)
				board.display_board_each_piece_and_side_separately();
			board.display_board();
			break;
		}
		
		board.make_move(result.best_move);
		//minmax calls
		std::cout << "best move: " << chess_notation2(result.best_move.move) << " score: " << result.score << "\n";
		std::cout << total_time << " seconds\n";
		std::cout << Engine::minmax_calls_count << std::endl;
		Engine::minmax_calls_count = 0;
		if (advance_print)
			board.display_board_each_piece_and_side_separately();
		board.display_board();
		board.display_pieces_counts();
		//read opponent move and make it on the board
		
		/*std::string opponent_move_str;
		std::cout << "opponent move: ";
		std::cin >> opponent_move_str;
		SimpleMove opponent_move = create_simple_move(opponent_move_str);
		board.make_move(opponent_move, get_move_type_from_squares(&board, opponent_move & 0x3F, opponent_move >> 6));
		board.display_board();*/

		//std::cout << move_to_string(result.best_move.move) << std::endl;
	}
}


void play_against_human(int n, uint8_t depth, bool advance_print = false, float time_per_move = 0, std::string fen = "")
{
	Board board;
	board.initialize_board();

	Engine engine(&board);


	if (fen != "")
		board.load_fen(fen);


	board.display_board();
	std::cout << depth << std::endl;
	SearchResult result;
	for (int i = 0; i < n; ++i)
	{
		double total_time = 0;
		auto start = std::chrono::high_resolution_clock::now();
		int j = 0;
		for (; time_per_move != 0 || j <= depth; ++j)
		{
			start = std::chrono::high_resolution_clock::now();
			result = engine.minimax_init(j);
			std::cout << "\rdepth: " << j << " completed";
			std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start;
			total_time += elapsed.count();
			if (time_per_move != 0 && total_time > time_per_move)
				break;
		}
		std::cout << std::endl;

		//debug only
		if (result.best_move.move >= 4096)
		{
			std::cout << "error: best move has invalid move value: " << result.best_move.move << std::endl;
			if (advance_print)
				board.display_board_each_piece_and_side_separately();
			board.display_board();
			std::abort();
		}
		//
		if (result.best_move == Move())
		{
			std::cout << "the game ended" << std::endl;
			if (advance_print)
				board.display_board_each_piece_and_side_separately();
			board.display_board();
			break;
		}

		board.make_move(result.best_move);
		//minmax calls
		std::cout << "best move: " << chess_notation2(result.best_move.move) << " score: " << result.score << "\n";
		std::cout << total_time << " seconds\n";
		std::cout << Engine::minmax_calls_count << std::endl;
		Engine::minmax_calls_count = 0;
		if (advance_print)
			board.display_board_each_piece_and_side_separately();
		board.display_board();


		//read opponent move and make it on the board

		std::string opponent_move_str;
		std::cout << "opponent move: ";
		std::cin >> opponent_move_str;
		SimpleMove opponent_move = create_simple_move(opponent_move_str);
		board.make_move(opponent_move, get_move_type_from_squares(&board, opponent_move & 0x3F, opponent_move >> 6));
		board.display_board();

		//std::cout << move_to_string(result.best_move.move) << std::endl;
	}
}



int main()
{
	//Board board = Board();
	//board.initialize_board();
	//board.make_move(Move(create_simple_move("a2a3"), QUIET_PAWN));
	//board.make_move(Move(create_simple_move("h7h6"), QUIET_PAWN));
	//board.make_move(Move(create_simple_move("a7a6"), CAPTURE_WITH_ROOK));



	//Board board = Board();
	//board.initialize_board();
	//board.load_fen("8/R5pk/5r1p/5P2/5KP1/8/8/8 b - - 32 81");
	//board.make_move(Move(create_simple_move("f6a6"), QUIET_ROOK));
	//board.make_move(Move(create_simple_move("a7a6"), CAPTURE_WITH_ROOK));
	////board.make_move(Move((6 * 8) | 0 << 6, QUIET_ROOK));
	////board.make_move(Move(54 | 61 << 6, QUIET_KING));
	//board.display_board();
	//board.initial_perft(2);
	run_unit_tests_with_perft(10000, 4, true);


	//int n = 4;
	//auto start = std::chrono::high_resolution_clock::now();
	//uint64_t searched_nodes = perft(n);
	//auto end = std::chrono::high_resolution_clock::now(); // End time
	//std::chrono::duration<double> elapsed = end - start;
	//std::cout << "perft(" << n << ") searched nodes: "<< searched_nodes <<" time: " << elapsed.count() << " seconds\n";

	//self_play(100, 8, false, 0, "8/8/8/8/B3nn2/q7/pp6/1k2K2R w K - 0 1");
	//self_play(100, 7, false);

	//play_against_human(10000, 8, false, 6, "r3kb1r/p1pb2pp/p2p1p1n/3Pp3/4P3/P1qQ1N2/1PP2PPP/1RB2RK1 w kq - 0 18");

	//Board board;
	//board.initialize_board();
	//auto start = std::chrono::high_resolution_clock::now();
	//board.perft(n);
	//auto end = std::chrono::high_resolution_clock::now(); // End time
	//uint64_t searched_nodes = board.perft_nodes_searched;
	//std::chrono::duration<double> elapsed = end - start;
	//std::cout << "perft(" << n << ") searched nodes: " << searched_nodes << " time: " << elapsed.count() << " seconds\n";


	/*Board board;
	board.initialize_board();
	std::cout << "zobrist key of initial position: " << board.zobrist_key << std::endl;
	board.se.calculate_score(false);
	std::cout << "initial position score: " << board.se.score << std::endl;
	std::cout << "e2e4 move: " << create_simple_move("e2e4") << std::endl;
	return 0;*/

	//Engine engine(&board);
	//constexpr int depth = 8;


	////board.se.calculate_score(false);
	////std::cout << board.se.score << std::endl;
	//
	//
	///*int16_t pst_black[6][64];
	//std::memcpy(pst_black, StaticEvaluation::pst_table_white, sizeof(pst_black));
	//for (int piece = PAWN; piece <= KING; ++piece)
	//{
	//	for (int square = 0; square < 64; ++square)
	//	{
	//		int rank = square / 8;
	//		int file = square % 8;
	//		int mirrored_rank = 7 - rank;
	//		int mirrored_square = mirrored_rank * 8 + file;
	//		pst_black[piece][square] = StaticEvaluation::pst_table_white[piece][mirrored_square];
	//	}
	//}
	////print
	//for (int piece = PAWN; piece <= KING; ++piece)
	//{
	//	std::cout << "{";
	//	for (int square = 0; square < 64; ++square)
	//	{
	//		std::cout << pst_black[piece][square] << ", ";
	//	}
	//	std::cout << "},\n";
	//}
	//return 0;*/


	////std::vector<std::string> moves_to_play = std::vector<std::string>();
	////moves_to_play.push_back("e2e4");


	///*for (int i = 0; i < moves_to_play.size(); ++i)
	//{
	//	SimpleMove move = create_simple_move(moves_to_play[i]);
	//	board.make_move(move, get_move_type_from_squares(&board, move & 0x3F, move >> 6));
	//	board.display_board();
	//}*/
	//board.display_board();
	//std::cout << depth << std::endl;
	//for (int i = 0; i < 100; ++i)
	//{
	//	double total_time = 0;
	//	auto start = std::chrono::high_resolution_clock::now();
	//	int j = 0;
	//	for (;true; ++j)
	//	{
	//		start = std::chrono::high_resolution_clock::now();
	//		engine.minmax_init(j);
	//		std::cout << "\rdepth: " << j << " completed";
	//		std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start;
	//		total_time += elapsed.count();
	//		if (total_time > 2)
	//			break;
	//	}
	//	start = std::chrono::high_resolution_clock::now();
	//	SearchResult result = engine.minmax_init(j);
	//	std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start;
	//	total_time += elapsed.count();
	//	std::cout << "\rdepth: " << j << " completed\n";


	//	//auto end = std::chrono::high_resolution_clock::now(); // End time
	//	//std::chrono::duration<double> elapsed = end - start;


	//	//debug only
	//	if (result.best_move.move >= 4096)
	//	{
	//		board.display_board();
	//		std::abort();
	//	}
	//	//
	//	if (result.best_move == Move()) break;
	//	board.make_move(result.best_move);
	//	//minmax calls
	//	std::cout << "best move: " << chess_notation2(result.best_move.move) << " score: " << result.score << "\n";
	//	std::cout << total_time << " seconds\n";
	//	std::cout << Engine::minmax_calls_count << std::endl;
	//	Engine::minmax_calls_count = 0;
	//	board.display_board();
	//	//read opponent move and make it on the board
	//	
	//	/*std::string opponent_move_str;
	//	std::cout << "opponent move: ";
	//	std::cin >> opponent_move_str;
	//	SimpleMove opponent_move = create_simple_move(opponent_move_str);
	//	board.make_move(opponent_move, get_move_type_from_squares(&board, opponent_move & 0x3F, opponent_move >> 6));
	//	board.display_board();*/

	//	//std::cout << move_to_string(result.best_move.move) << std::endl;
	//}
	//std::cout << move_to_string2(engine.minmax_init(7).best_move.move) << std::endl;


	return 0;
}