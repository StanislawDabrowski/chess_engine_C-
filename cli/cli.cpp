#include <iostream>
#include <unordered_map>
#include <functional>
#include <string>
#include <sstream>
#include <chrono>
#include "../chess engine C++/Engine.h"






Board board;
Engine engine(&board);


std::string chess_notation(Move full_move)
{
	SimpleMove move = full_move.move;
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
	//check for promotions
	switch (full_move.move_type)
	{
	case MoveType::QUEEN_PROMOTION:
		notation += "q";
		break;
	case MoveType::ROOK_PROMOTION:
		notation += "r";
		break;
	case MoveType::BISHOP_PROMOTION:
		notation += "b";
		break;
	case MoveType::KNIGHT_PROMOTION:
		notation += "n";
		break;
	default:
		break;
	}
	return notation;
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

MoveType get_move_type_from_squares(Board* board, uint8_t from_square, uint8_t to_square, char promotion)
{
	PieceType piece = get_piece_on_square(board, from_square);
	PieceType target_piece = get_piece_on_square(board, to_square);
	if (piece == PAWN)
	{
		if (to_square == from_square + 8 || to_square == from_square - 8 || to_square == from_square + 16 || to_square == from_square - 16)//quiet move
		{
			if (to_square >= 56 || to_square < 8)//promotion
			{
				switch (promotion)
				{
				case 'q':
					return QUEEN_PROMOTION;
				case 'r':
					return ROOK_PROMOTION;
				case 'b':
					return BISHOP_PROMOTION;
				case 'n':
					return KNIGHT_PROMOTION;
				}
			}
			else
				return QUIET_PAWN;
		}
		else//capture
		{
			if (to_square >= 56 || to_square < 8)//promotion with capture
			{
				switch (promotion)
				{
				case 'q':
					return QUEEN_PROMOTION;
				case 'r':
					return ROOK_PROMOTION;
				case 'b':
					return BISHOP_PROMOTION;
				case 'n':
					return KNIGHT_PROMOTION;
				}
			}
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

Move string_to_move(Board* board, std::string move_str)
{
	Move move;
	move.move = create_simple_move(move_str);
	uint8_t from_square = move.move & 0x3F;
	uint8_t to_square = (move.move >> 6);
	move.move_type = get_move_type_from_squares(board, from_square, to_square, move_str[4]);
	return move;
}

bool is_move_legal(Move move)
{
	board.mg.generate_pseudo_legal_moves_with_category_ordering();
	board.mg.filter_pseudo_legal_moves();
	std::vector<Move> legal_moves_vec = board.mg.get_legal_moves();
	for (const Move& legal_move : legal_moves_vec)
	{
		if (legal_move == move)
		{
			return true;
		}
	}
	return false;
}

bool is_move_legal(std::string move)
{
	return is_move_legal(string_to_move(&board, move));
}

void invalid_syntax_message()
{
	std::cout << "Invalid command syntax. Type 'help' or '?' for a list of commands." << std::endl;
}


void help_command_function(std::vector<std::string> args)
{
	std::cout<<R"(list of commands and their description
	help / ? - prints this help message
	position - sets specificed position on the board
		startpos - the initial position of the chess game
		fen <fen string> -sets the position given in fen
	go - runs minmax on the set position. Prints best move in format <move from><move to> (e.g.e2e4(pawn move), e1g1(white kingside castle)) in the next line prints the evaluation
		depth <depth> - runs at minimum to the specified depth
		min_time <time in seconds> - run for the minimum of the specified time
		max_time <time in seconds> - tries to run for the maximum of the specified time
		auto_make - automaticly makes the best move after the search(output is the same)
	move <move> - makes the specified move on the board
	undo - undoes the last move made
	is_move_legal <move> - checks if the specified move is legal in the current position
	fen - prints fen of the current position
	d - displays / prints board
		debug - print sthe board in debug mode
	clear_TT - sets TT entries value to default "empty" values
	exit / quit / e / q - quits the program
)";
}

void display_board_command_function(std::vector<std::string> args)
{
	board.display_board(std::cout);
}

void exit_command_function(std::vector<std::string> args)
{
	std::cout << "Exiting program." << std::endl;
	exit(0);
}

void position_command_function(std::vector<std::string> args)
{
	std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";//initial position
	if (args.size() == 0)
	{
		invalid_syntax_message();
		return;
	}
	if (args[0] != "startpos")
	{
		if (args[0] != "fen")
		{
			invalid_syntax_message();
			return;
		}
		if (args.size() < 2)
		{
			invalid_syntax_message();
			return;
		}
		fen = args[1];
	}

	if (!board.load_fen(fen))
	{
		std::cout << "Invalid FEN string.\nSetting initial board position" << std::endl;
	}

}

void go_command_function(std::vector<std::string> args)
{
	int depth = -1;
	int min_time = -1;
	int max_time = -1;
	int auto_make = -1;
	for (int i = 0; i < args.size(); ++i)
	{
		if (args[i] == "depth")
		{
			if (i + 1 >= args.size())
			{
				invalid_syntax_message();
				return;
			}
			try {
				depth = std::stoi(args[i + 1]);
			}
			catch (const std::invalid_argument&) 
			{
				invalid_syntax_message();
				return;
			}
			++i;
		}
		else if (args[i] == "min_time")
		{
			if (i + 1 >= args.size())
			{
				invalid_syntax_message();
				return;
			}
			try {
				min_time = std::stoi(args[i + 1]);
			}
			catch (const std::invalid_argument&)
			{
				invalid_syntax_message();
				return;
			}
			++i;
		}
		else if (args[i] == "max_time")
		{
			if (i + 1 >= args.size())
			{
				invalid_syntax_message();
				return;
			}
			try {
				max_time = std::stoi(args[i + 1]);
			}
			catch (const std::invalid_argument&)
			{
				invalid_syntax_message();
				return;
			}
			++i;
		}
		else if (args[i] == "auto_make")
		{
			auto_make = 1;
		}
		else
		{
			invalid_syntax_message();
			return;
		}
	}
	if (depth == -1 && min_time == -1)
	{
		invalid_syntax_message();
		return;
	}


	SearchResult result;
	double total_time = 0;
	std::chrono::duration<double> elapsed;
	auto start = std::chrono::high_resolution_clock::now();

	int i = 0;
	for (; i <= depth; ++i)
	{
		result = engine.minimax_init(i);
		elapsed = std::chrono::high_resolution_clock::now() - start;
		if (max_time != -1 && elapsed.count() > max_time)
			break;
	}
	if (min_time != -1)
	{
		for (; elapsed.count() < min_time; ++i)
		{
			result = engine.minimax_init(i);
			elapsed = std::chrono::high_resolution_clock::now() - start;
		}
	}

	if (auto_make == 1)
	{
		board.make_move(result.best_move);
	}

	std::cout << chess_notation(result.best_move) << std::endl;
	std::cout << result.score << std::endl;
}

void fen_command_function(std::vector<std::string> args)
{
	std::cout << board.get_fen() << std::endl;
}

void move_command_function(std::vector<std::string> args)
{
	if (args.size() < 1)
	{
		invalid_syntax_message();
		return;
	}
	std::string move_str = args[0];
	Move move;
	try {
		SimpleMove simple_move = create_simple_move(move_str);
		uint8_t from_square = simple_move & 0b111111;
		uint8_t to_square = (simple_move >> 6);
		MoveType move_type = get_move_type_from_squares(&board, from_square, to_square, move_str[4]);
		move = Move{ simple_move, move_type };
	}
	catch (const std::runtime_error& e)
	{
		std::cout << "Invalid move format." << std::endl;
		return;
	}
	if (is_move_legal(move))
		board.make_move(move);
	else
		std::cout << "illegal move" << std::endl;
}

void undo_command_function(std::vector<std::string> args)
{
	board.undo_move();
}

void clear_TT_command_function(std::vector<std::string> args)
{
	engine.clear_TT();
}

void is_move_legal_command_function(std::vector<std::string> args)
{
	if (args.size() < 1)
	{
		invalid_syntax_message();
		return;
	}
	if (is_move_legal(args[0]))
		std::cout << "legal" << std::endl;
	else
	{
		std::cout << "illegal" << std::endl;
	}
}

std::vector<std::string> tokenize(const std::string& input)
{
	std::vector<std::string> tokens;
	std::string temp = "";

	bool in_quotes = false;

	for (int i = 0; i < input.size(); ++i)
	{
		char c = input[i];
		if (c == '\"')
		{
			in_quotes = !in_quotes;
			continue;
		}
		if (c == ' ' && !in_quotes)
		{
			if (!temp.empty())
			{
				tokens.push_back(temp);
				temp.clear();
			}
		}
		else
		{
			temp += c;
		}
	}
	if (!temp.empty())
	{
		tokens.push_back(temp);
	}

	return tokens;
}



int main()
{
	board.initialize_board();


	std::unordered_map<std::string, std::function<void(std::vector<std::string>)>> commands_functions = {
		{"help", help_command_function},
		{"?", help_command_function},
		{"position", position_command_function},
		{"go", go_command_function},
		{"move", move_command_function},
		{"undo", undo_command_function},
		{"is_move_legal", is_move_legal_command_function},
		{"fen", fen_command_function},
		{"d", display_board_command_function},
		{"clear_TT", clear_TT_command_function},
		{"exit", exit_command_function},
		{"quit", exit_command_function},
		{"e", exit_command_function},
		{"q", exit_command_function}
	};

	

	std::string input_line;
	std::vector<std::string> tokens;
	while (true)
	{
		std::cout << ">>> ";
		//read the input line with cin
		
		std::getline(std::cin, input_line);
		tokens = tokenize(input_line);
		if (tokens.empty())
			continue;
		if (commands_functions.contains(tokens[0]))
		{
			commands_functions[tokens[0]](std::vector<std::string>(tokens.begin()+1, tokens.end()));
		}
		else
		{
			std::cout << "Unknown command: " << tokens[0] << std::endl;
			continue;
		}

	}



	return 0;
}