#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <algorithm>
#include "unit_tests.h"
#include "Board.h"
#include "Move.h"

struct test_position {
	std::string fen;
	std::vector<Move> pseudo_legal_moves;
	std::vector<Move> legal_moves;
};

void read_line(std::string line, test_position* tp)
{
	tp->fen.clear();
	tp->pseudo_legal_moves.clear();
	tp->legal_moves.clear();
	std::string token;
	std::istringstream lineStream(line);
	while (std::getline(lineStream, token, ';'))
	{
		if (tp->fen.empty()) {
			tp->fen = token;
		}
		else if (tp->pseudo_legal_moves.empty()) {
			// parse pseudo_legal_moves
			std::istringstream movesStream(token);
			std::string moveToken;
			while (std::getline(movesStream, moveToken, '/')) {
				uint16_t move = std::stoi(moveToken.substr(0, moveToken.find(',')));
				if (move >> 12)//is required to be zero
				{
					std::cout << "Move high bits set! move=" << move << std::endl;
					std::abort();
				}
				MoveType moveType = MoveType(std::stoi(moveToken.substr(moveToken.find(',') + 1)));
				tp->pseudo_legal_moves.push_back(Move(move, moveType));
			}
		}
		else if (tp->legal_moves.empty()) {
			// parse legal_moves
			std::istringstream movesStream(token);
			std::string moveToken;
			while (std::getline(movesStream, moveToken, '/')) {
				uint16_t move = std::stoi(moveToken.substr(0, moveToken.find(',')));
				if (move >> 12)//is required to be zero
				{
					std::cout << "Move high bits set! move=" << move << std::endl;
					std::abort();
				}
				MoveType moveType = MoveType(std::stoi(moveToken.substr(moveToken.find(',') + 1)));
				tp->legal_moves.push_back(Move(move, moveType));
			}
		}
	}
}

void read_line2(const std::string& line, test_position* tp) {
	size_t p1 = line.find(';');
	size_t p2 = line.find(';', p1 + 1);

	tp->fen.assign(line, 0, p1);

	auto parse_moves = [](const std::string& s, std::vector<Move>& out) {
		out.clear();
		size_t start = 0;
		while (start < s.size()) {
			size_t slash = s.find('/', start);
			size_t comma = s.find(',', start);
			if (comma == std::string::npos) break;

			uint16_t move = static_cast<uint16_t>(std::stoi(s.substr(start, comma - start)));
			MoveType mt = MoveType(std::stoi(s.substr(comma + 1, (slash == std::string::npos ? s.size() : slash) - comma - 1)));
			out.emplace_back(move, mt);

			if (slash == std::string::npos) break;
			start = slash + 1;
		}
		};

	parse_moves(line.substr(p1 + 1, p2 - p1 - 1), tp->pseudo_legal_moves);
	parse_moves(line.substr(p2 + 1), tp->legal_moves);
}


bool equal_unordered_sorted(std::vector<Move> a, std::vector<Move> b) {
	if (a.size() != b.size()) return false;
	std::sort(a.begin(), a.end());
	std::sort(b.begin(), b.end());
	return a == b;
}


void test_equal_unordered_sorted() {
	std::vector<Move> v1 = { Move(0x01, MoveType::QUIET_BISHOP), Move(0x02, MoveType::CAPTURE_WITH_BISHOP) };
	std::vector<Move> v2 = { Move(0x02, MoveType::CAPTURE_WITH_BISHOP), Move(0x01, MoveType::QUIET_BISHOP) };
	std::vector<Move> v3 = { Move(0x01, MoveType::QUIET_BISHOP) };
	std::vector<Move> v4 = { Move(0x01, MoveType::QUIET_BISHOP), Move(0x03, MoveType::QUIET_BISHOP) };

	// Should be true: same elements, different order
	if (!equal_unordered_sorted(v1, v2))
		std::cout << "Test 1 failed\n";

	// Should be false: different size
	if (equal_unordered_sorted(v1, v3))
		std::cout << "Test 2 failed\n";

	// Should be false: same size, different elements
	if (equal_unordered_sorted(v1, v4))
		std::cout << "Test 3 failed\n";

	// Should be true: empty vectors
	if (!equal_unordered_sorted(std::vector<Move>(), std::vector<Move>()))
		std::cout << "Test 4 failed\n";
}

std::string chess_notation(uint16_t move)//convert move to chess notation for easier debugging
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

std::string move_type_name(MoveType move_type)
{
	switch (move_type)
	{
	case QUEEN_PROMOTION:
		return "QUEEN PROMOTION";
		break;
	case KNIGHT_PROMOTION:
		return "KNIGHT PROMOTION";
		break;
	case BISHOP_PROMOTION:
		return "BISHOP PROMOTION";
		break;
	case ROOK_PROMOTION:
		return "ROOK PROMOTION";
		break;
	case CAPTURE_WITH_PAWN:
		return "CAPTURE WITH PAWN";
		break;
	case CAPTURE_WITH_KNIGHT:
		return "CAPTURE WITH KNIGHT";
		break;
	case CAPTURE_WITH_BISHOP:
		return "CAPTURE WITH BISHOP";
		break;
	case CAPTURE_WITH_ROOK:
		return "CAPTURE WITH ROOK";
		break;
	case CAPTURE_WITH_QUEEN:
		return "CAPTURE WITH QUEEN";
		break;
	case CAPTURE_WITH_KING:
		return "CAPTURE WITH KING";
		break;
	case CASTLE:
		return "CASTLE";
		break;
	case QUIET_KNIGHT:
		return "QUIET KNIGHT";
		break;
	case QUIET_BISHOP:
		return "QUIET BISHOP";
		break;
	case QUIET_ROOK:
		return "QUIET ROOK";
		break;
	case QUIET_QUEEN:
		return "QUIET QUEEN";
		break;
	case QUIET_KING:
		return "QUIET KING";
		break;
	case QUIET_PAWN:
		return "QUIET PAWN";
		break;
	default:
		return "UNKNOWN MOVE TYPE";
		break;
	}
}



int run_unit_tests(const int NUM_OF_POSITIONS_TO_TEST)//set to -1 to test all positions in the file (probably 5'423'663 positions)
{
	test_equal_unordered_sorted();
	//.csv file format:
	//fen;pseudo_legal_moves;legal_moves
	Board board = Board();
	
	bool DISPLAY_FAILED_OUTPUTS = true;


	test_position tp;

	std::ifstream file("C:/Users/Avalfortz/source/repos/chess engine C++/tests/lichess_db_puzzle_out.csv");

	if (!file.is_open()) {
		std::cerr << "File not found\n";
		return 1;
	}

	std::string line;
	std::vector<test_position> test_positions;
	test_positions.reserve(NUM_OF_POSITIONS_TO_TEST <= 0 ? 5'423'663 : NUM_OF_POSITIONS_TO_TEST);

	std::cout << "reading started" << std::endl;
	std::getline(file, line);

	int loaded_positions = 0;
	for (int i = 0; !file.eof(); ++i)
	{
		if (i == NUM_OF_POSITIONS_TO_TEST) break;
		std::getline(file, line);
		if (line.empty()) continue;
		read_line2(line, &tp);
		test_positions.push_back(tp);
		if (NUM_OF_POSITIONS_TO_TEST > 0)
		{
			if (i % 10'000 == 0)
				std::cout << "\rReading file: " << (i * 100 / NUM_OF_POSITIONS_TO_TEST) << "% completed" << std::flush;
		}
		else
			if (i % 10'000 == 0)
				std::cout << "\rReading file: " << (i * 100 / 5'423'663) << "% completed" << std::flush;
		if (i == NUM_OF_POSITIONS_TO_TEST - 1)
			std::cout << std::endl;
		++loaded_positions;
	}
	file.close();




	std::cout << "testing started" << std::endl;
	std::vector<Move> moves;
	uint64_t zobrist_key1;
	uint64_t zobrist_key2;
	for (int i = 0; i < test_positions.size(); ++i)
	{
		if (!board.load_fen(test_positions[i].fen))
		{
			std::cout << "failed to load fen at position with index: " << i << std::endl;
			continue;
		}
		//board.calculate_zobrist_key();

		board.mg.generate_pseudo_legal_moves_with_category_ordering2();
		moves = board.mg.get_pseudo_legal_moves2();
		//board.display_board();
		if (!equal_unordered_sorted(test_positions[i].pseudo_legal_moves, moves))
		{
			std::vector<Move> v = moves;
			std::sort(v.begin(), v.end());

			std::vector<Move> duplicates;
			for (size_t i = 1; i < v.size(); i++) {
				if (v[i] == v[i - 1] && (duplicates.empty() || !(duplicates.back() == v[i]))) {
					duplicates.push_back(v[i]);
				}
			}

			if (!duplicates.empty()) {
				std::cout << "Duplicates found: ";
				for (Move x : duplicates) std::cout << "(" << chess_notation(x.move) << "," << int(x.move_type) << ") ";
				std::cout << "\n";
			}

			std::cout << "pseudo legal moves don't match in position with index: " << i << std::endl;
			if (DISPLAY_FAILED_OUTPUTS)
			{
				//first printe expected moves which are not generated then generated moves which are not expected
				std::cout << "not generated moves: ";
				bool any_moves_printed = false;
				for (const Move& expected_move : test_positions[i].pseudo_legal_moves)
				{
					if (std::find(moves.begin(), moves.end(), expected_move) == moves.end())
					{
						std::cout << "(" << chess_notation(expected_move.move) << "," << move_type_name(expected_move.move_type) << ") ";
						any_moves_printed = true;
					}
				}
				if (!any_moves_printed)
					std::cout << "none";
				std::cout << std::endl << "unexpected generated moves: ";
				any_moves_printed = false;
				for (const Move& generated_move : moves)
				{
					if (std::find(test_positions[i].pseudo_legal_moves.begin(), test_positions[i].pseudo_legal_moves.end(), generated_move) == test_positions[i].pseudo_legal_moves.end())
					{
						std::cout << "(" << chess_notation(generated_move.move) << "," << move_type_name(generated_move.move_type) << ") ";
						any_moves_printed = true;
					}
				}
				if (!any_moves_printed)
					std::cout << "none";
				std::cout << std::endl;
				board.display_board();
			}
		}
		board.mg.filter_pseudo_legal_moves2();
		moves = board.mg.get_legal_moves2();
		if (!equal_unordered_sorted(moves, test_positions[i].legal_moves))
		{
			std::cout << "legal moves don't match in position with index: " << i << std::endl;
			if (DISPLAY_FAILED_OUTPUTS)
			{
				//first printe expected moves which are not generated then generated moves which are not expected
				std::cout << "not generated moves: ";
				bool any_moves_printed = false;
				for (const Move& expected_move : test_positions[i].legal_moves)
				{
					if (std::find(moves.begin(), moves.end(), expected_move) == moves.end())
					{
						std::cout << "(" << chess_notation(expected_move.move) << "," << move_type_name(expected_move.move_type) << ") ";
						any_moves_printed = true;
					}
				}
				if (!any_moves_printed)
					std::cout << "none";
				std::cout << std::endl << "unexpected generated moves: ";
				for (const Move& generated_move : moves)
				{
					if (std::find(test_positions[i].legal_moves.begin(), test_positions[i].legal_moves.end(), generated_move) == test_positions[i].legal_moves.end())
					{
						std::cout << "(" << chess_notation(generated_move.move) << "," << move_type_name(generated_move.move_type) << ") ";
						any_moves_printed = true;
					}
				}
				if (!any_moves_printed)
					std::cout << "none";
				std::cout << std::endl;
				board.display_board();
			}
		}
		for (int j = 0; j < test_positions[i].pseudo_legal_moves.size(); ++j)
		{
			board.calculate_zobrist_key();
			zobrist_key1 = board.zobrist_key;
			board.make_move(test_positions[i].pseudo_legal_moves[j].move, test_positions[i].pseudo_legal_moves[j].move_type);
			zobrist_key2 = board.zobrist_key;
			board.calculate_zobrist_key();
			if (zobrist_key2 != board.zobrist_key)
			{
				std::cout << "zobrist key does not match after making move at position of index: " << i << " at pseudo legal move of index: " << j << std::endl;
				if (DISPLAY_FAILED_OUTPUTS)
				{
					//print the move and borad state
					std::cout << "move: (" << chess_notation(test_positions[i].pseudo_legal_moves[j].move) << "," << move_type_name(test_positions[i].pseudo_legal_moves[j].move_type) << ")\n";
					board.display_board();
				}
			}
			board.undo_move();
			//board.calculate_zobrist_key();
			if (zobrist_key1 != board.zobrist_key)
			{
				std::cout << "zobrist key does not match at position of index: " << i << " at pseudo legal move of index: " << j << std::endl;
				if (DISPLAY_FAILED_OUTPUTS)
				{
					//print the move and borad state
					std::cout << "move: (" << chess_notation(test_positions[i].pseudo_legal_moves[j].move) << "," << move_type_name(test_positions[i].pseudo_legal_moves[j].move_type) << ")\n";
					board.display_board();
				}
			}
		}
		if (i % 10'000 == 0) {
			std::cout << "\rTesting positions: " << (i * 100 / test_positions.size()) << "% completed" << std::flush;
		}
	}
	std::cout << std::flush;





	return 0;
}