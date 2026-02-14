// benchmark.cpp
// Compile with: g++ -O3 -std=c++17 benchmark.cpp -o benchmark
// Assumes the provided headers and their implementations are available.

#include <chrono>
#include <string>
#include <cctype>
#include <cstdint>

using clk = std::chrono::steady_clock;
using ns = std::chrono::nanoseconds;


int s2(const std::string& square) {
    if (square.length() != 2)
        return -1; // invalid input

    char file = std::toupper(square[0]); // 'A' to 'H'
    char rank = square[1]; // '1' to '8'

    if (file < 'A' || file > 'H' || rank < '1' || rank > '8')
        return -1; // invalid input

    int fileIndex = file - 'A';
    int rankIndex = rank - '1';

    return rankIndex * 8 + fileIndex; // index from bottom-left
}

uint16_t to_move(uint8_t from, uint8_t to)
{
    return from | (to << 6);
}



//int main()
//{
//    // parameters (adjust if needed)
//    const int MAKE_UNDO_ITERS = 10000000; // make+undo loop iterations
//    const int PSEUDO_ITERS = 10000000;  // pseudo-legal generation iterations
//    const int STATIC_ITERS = 1000000;  // static evaluation iterations
//    const int LEGAL_ITERS = 1000000;   // legal moves generation iterations
//	const int PSEUDO_LEGAL_ATTACKS_ITERS = 1000000; // pseudo-legal attacks generation iterations
//	const int GET_PEICE_TYPE_ITERS = 10000000; // get_piece_type iterations
//
//    Board board;
//    board.initialize_board();
//
//    board.make_move(to_move(s2("e2"), s2("e4")), QUIET_PAWN);
//    board.make_move(to_move(s2("e7"), s2("e5")), QUIET_PAWN);
//    board.make_move(to_move(s2("g1"), s2("f3")), QUIET_KNIGHT);
//    board.make_move(to_move(s2("b8"), s2("c6")), QUIET_KNIGHT);
//    board.make_move(to_move(s2("b1"), s2("c3")), QUIET_KNIGHT);
//    board.make_move(to_move(s2("g8"), s2("f6")), QUIET_KNIGHT);
//    board.make_move(to_move(s2("f1"), s2("b5")), QUIET_BISHOP);
//    board.make_move(to_move(s2("f8"), s2("b4")), QUIET_BISHOP);
//
//    // prepare objects
//    StaticEvaluation se(&board);
//
//    // Ensure we have at least one move to test make/undo
//    board.mg.generate_pseudo_legal_moves_with_ordering();
//    /*std::vector<Move> initial_legal = board.mg.get_legal_moves();
//    uint16_t testMove;
//    if (!initial_legal.empty()) testMove = initial_legal[0];
//    else testMove = Move(); // fallback default move*/
//	uint16_t test_move = to_move(s2("f3"), s2("e5")); // white knight takes black pawn
//	MoveType test_move_type = CAPTURE_WITH_KNIGHT;
//
//    // 1) make_move + undo_move benchmark
//    {
//        auto t0 = clk::now();
//        for (int i = 0; i < MAKE_UNDO_ITERS; ++i)
//        {
//            board.make_move(test_move, test_move_type);
//            board.undo_move();
//        }
//        auto t1 = clk::now();
//        ns dur = std::chrono::duration_cast<ns>(t1 - t0);
//        double avg_ns = double(dur.count()) / MAKE_UNDO_ITERS;
//        std::cout << "Make+Undo iterations: " << MAKE_UNDO_ITERS << '\n';
//        std::cout << "Total time: " << std::fixed << std::setprecision(3)
//            << (dur.count() / 1e6) << " ms.  Avg per make+undo: " << avg_ns << " ns\n\n";
//    }
//
//    // 2) generate_pseudo_legal_moves benchmark
//    {
//        // warmup
//        board.mg.generate_pseudo_legal_moves_with_ordering();
//
//        auto t0 = clk::now();
//        for (int i = 0; i < PSEUDO_ITERS; ++i)
//        {
//            board.mg.generate_pseudo_legal_moves_with_ordering();
//        }
//        auto t1 = clk::now();
//        ns dur = std::chrono::duration_cast<ns>(t1 - t0);
//        double avg_ns = double(dur.count()) / PSEUDO_ITERS;
//        std::cout << "Generate pseudo-legal moves iterations: " << PSEUDO_ITERS << '\n';
//        std::cout << "Total time: " << std::fixed << std::setprecision(3)
//            << (dur.count() / 1e6) << " ms.  Avg per call: " << avg_ns << " ns\n\n";
//    }
//
//    // 3) static evaluation benchmark
//    {
//        // We call generate_pseudo_legal_moves before each static evaluation as required.
//        // warmup
//        //board.mg.generate_pseudo_legal_moves(static_cast<uint8_t>(board.side_to_move));
//        se.calculate_score(false);
//
//        auto t0 = clk::now();
//        for (int i = 0; i < STATIC_ITERS; ++i)
//        {
//            //board.mg.generate_pseudo_legal_moves(static_cast<uint8_t>(board.side_to_move));
//            se.calculate_score(false);
//        }
//        auto t1 = clk::now();
//        ns dur = std::chrono::duration_cast<ns>(t1 - t0);
//        double avg_ns = double(dur.count()) / STATIC_ITERS;
//        std::cout << "Static evaluation iterations: " << STATIC_ITERS << '\n';
//        std::cout << "Total time (pseudo+static each iter): " << std::fixed << std::setprecision(3)
//            << (dur.count() / 1e6) << " ms.  Avg per (pseudo+static): " << avg_ns << " ns\n\n";
//    }
//
//    // 4) generate legal moves benchmark
//    {
//        // legal moves require pseudo-legal generation first.
//        board.mg.generate_pseudo_legal_moves_with_ordering();
//        auto warm = board.mg.get_legal_moves();
//
//        auto t0 = clk::now();
//		uint8_t side = static_cast<uint8_t>(board.side_to_move);
//        for (int i = 0; i < LEGAL_ITERS; ++i)
//        {
//            board.mg.generate_pseudo_legal_moves_with_ordering();
//            board.mg.filter_pseudo_legal_moves();
//        }
//        auto t1 = clk::now();
//        ns dur = std::chrono::duration_cast<ns>(t1 - t0);
//        double avg_ns = double(dur.count()) / LEGAL_ITERS;
//        std::cout << "Generate legal moves iterations: " << LEGAL_ITERS << '\n';
//        std::cout << "Total time (pseudo+get_legal_moves each iter): " << std::fixed << std::setprecision(3)
//            << (dur.count() / 1e6) << " ms.  Avg per (pseudo+legal): " << avg_ns << " ns\n\n";
//    }
//
//    //5) generate pseudo-legal attacks benchmark
//    {
//        // warmup
//        board.mg.generate_pseudo_legal_attacks(static_cast<uint8_t>(board.side_to_move));
//        auto t0 = clk::now();
//        for (int i = 0; i < PSEUDO_LEGAL_ATTACKS_ITERS; ++i)
//        {
//            board.mg.generate_pseudo_legal_attacks(static_cast<uint8_t>(board.side_to_move));
//        }
//        auto t1 = clk::now();
//        ns dur = std::chrono::duration_cast<ns>(t1 - t0);
//        double avg_ns = double(dur.count()) / PSEUDO_LEGAL_ATTACKS_ITERS;
//        std::cout << "Generate pseudo-legal attacks iterations: " << PSEUDO_LEGAL_ATTACKS_ITERS << '\n';
//        std::cout << "Total time: " << std::fixed << std::setprecision(3)
//            << (dur.count() / 1e6) << " ms.  Avg per call: " << avg_ns << " ns\n\n";
//	}
//
//    /*//6) get_piece_type benchmark
//    {
//        // warmup
//        board.mg.get_piece_type(board.P[0], 0, 0);
//        auto t0 = clk::now();
//        for (int i = 0; i < GET_PEICE_TYPE_ITERS; ++i)
//        {
//            board.mg.get_piece_type(board.P[0], i % 64, i % 2);
//        }
//        auto t1 = clk::now();
//        ns dur = std::chrono::duration_cast<ns>(t1 - t0);
//        double avg_ns = double(dur.count()) / GET_PEICE_TYPE_ITERS;
//        std::cout << "get_piece_type iterations: " << GET_PEICE_TYPE_ITERS << '\n';
//        std::cout << "Total time: " << std::fixed << std::setprecision(3)
//            << (dur.count() / 1e6) << " ms.  Avg per call: " << avg_ns << " ns\n\n";
//	}*/
//   
//
//    return 0;
//}
