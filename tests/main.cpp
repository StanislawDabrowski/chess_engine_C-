#include <iostream>
#include "Board.h"
#include "Move.h"
#include "unit_tests.h"
#include "performance_tests.h"
#include <Board.cpp>
#include <string>
#include <chrono>


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


int main()
{
	run_unit_tests(100000);

	/*int n = 7;
	//for (int i = 0;i<=n;++i)
		//std::cout<<perft(i)<<std::endl;
	auto start = std::chrono::high_resolution_clock::now();
	perft(n);
	auto end = std::chrono::high_resolution_clock::now(); // End time
	std::chrono::duration<double> elapsed = end - start;
	std::cout << "perft(" << n << ") time: " << elapsed.count() << " seconds\n";*/


	return 0;
}