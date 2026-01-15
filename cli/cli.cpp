#include <iostream>
#include <unordered_map>
#include <functional>
#include <string>
#include <sstream>
#include <chrono>
#include "../chess engine C++/Engine.h"






Board board;
Engine engine(&board);


std::string chess_notation(Move full_move)//convert move to chess notation for easier debugging
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


void invalid_syntax_message()
{
	std::cout << "Invalid command syntax. Type 'help' or '?' for a list of commands." << std::endl;
}


void help_command_function(std::vector<std::string> args)
{
	std::cout<<R"(list of commands and their description
	help / ? -prints this help message
	position - sets specificed position on the board
		startpos - the initial position of the chess game
		fen <fen string> -sets the position given in fen
	best_move - runs minmax on the set position. Prints best move in format <move from><move to> (e.g.e2e4(pawn move), e1g1(white kingside castle)) in the next line prints the evaluation
		depth <depth> - runs at minimum to the specified depth
		min_time <time in seconds> - run for the minimum of the specified time
		max_time <time in seconds> - tries to run for the maximum of the specified time
			auto_make - automaticly makes the best move after the search(output is the same)
	is_checkmate - checks if the current position is a checkmate, prints "checkmate" or "not checkmate"
		fen - prints fen of the current position
	d - displays / prints board
		debug - print sthe board in debug mode
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

void best_move_command_function(std::vector<std::string> args)
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
	for (; i < depth; ++i)
	{
		result = engine.minmax_init(i);
		elapsed = std::chrono::high_resolution_clock::now() - start;
		if (max_time != -1 && elapsed.count() > max_time)
			break;
	}
	if (min_time != -1)
	{
		for (; elapsed.count() < min_time; ++i)
		{
			result = engine.minmax_init(i);
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
		{"best_move", best_move_command_function},
		{"d", display_board_command_function},
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