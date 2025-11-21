#include <limits>
#include "Engine.h"


Engine::Engine(Board* board)
	: board(board), se(board)
{
	tt = new TTEntry[tt_size];
}

Engine::~Engine()
{
	delete[] tt;
}

/*SearchResult Engine::find_best_move(uint8_t max_depth)
{
	return minmax_init(max_depth, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
}*/

/*SearchResult Engine::minmax_init(uint8_t depth, int16_t alpha, int16_t beta)
{
	//init zobrist key
	uint64_t zobrist_key = 0;
	//calculate initial zobrist key
	unsigned long square;
	for (int color = 0; color < 2; color++)
	{
		for (int piece = 0; piece < 6; piece++)
		{
			Bitboard pieces = board->P[piece][color];
			while (pieces)
			{
				
				_BitScanForward64(&square, pieces);
				zobrist_key ^= zobrist_pieces[color][piece][square];
				pieces &= pieces - 1;
			}
		}
	}
	//castling rights
	if (board->castling_rights & 1) zobrist_key ^= zobrist_castling[0];
	if (board->castling_rights & 2) zobrist_key ^= zobrist_castling[1];
	if (board->castling_rights & 4) zobrist_key ^= zobrist_castling[2];
	if (board->castling_rights & 8) zobrist_key ^= zobrist_castling[3];
	//en passant
	zobrist_key ^= zobrist_en_passant[board->en_passant_square];//if no en passant, en_passant==0 and the zobrist table at inedx 0 has 0 so it has no effect



	//check transposition table
	uint32_t zobrist_index = zobrist_key % tt_size;
	if (tt[zobrist_index].key == zobrist_key && tt[zobrist_key].depth >= depth)
	{		
		if (tt[zobrist_key].flag == EXACT)
			return SearchResult(tt[zobrist_index].score, tt[zobrist_index].best_move);
		else if (tt[zobrist_key].flag == ALPHA && tt[zobrist_key].score <= alpha)
			return SearchResult(alpha, tt[zobrist_index].best_move);
		else if (tt[zobrist_key].flag == BETA && tt[zobrist_key].score >= beta)
			return SearchResult(beta, tt[zobrist_index].best_move);
	}


	if (depth == 0)
	{
		se.calculate_score(false);
		return SearchResult(se.score, Move());
	}
	board->mg.generate_pseudo_legal_moves(board->side_to_move);
	board->mg.filter_pseudo_legal_moves();

	if (board->side_to_move)//black to move, minimizing player
	{
		int16_t min_eval = std::numeric_limits<int16_t>::max();
		Move best_move;
		Move m;
		for (int i = 0; i < board->mg.legal_moves_count; ++i)
		{
			m = board->mg.legal_moves[i];
			board->make_move(m);
			int16_t eval = minmax(depth - 1, alpha, beta, &zobrist_key);
			board->undo_move();
			if (eval < min_eval)
			{
				min_eval = eval;
				best_move = m;
			}
			beta = std::min(beta, eval);
			if (beta <= alpha)
				break;//alpha-beta pruning
		}
	}
	else//white to move, maximizing player
	{
		int16_t max_eval = std::numeric_limits<int16_t>::min();
		Move best_move;
		Move m;
		for (int i = 0; i < board->mg.legal_moves_count; ++i)
		{
			m = board->mg.legal_moves[i];
			board->make_move(m);
			int16_t eval = minmax(depth - 1, alpha, beta, &zobrist_key);
			board->undo_move();
			if (eval > max_eval)
			{
				max_eval = eval;
				best_move = m;
			}
			alpha = std::max(alpha, eval);
			if (beta <= alpha)
				break;//alpha-beta pruning
		}
		return SearchResult(max_eval, best_move);
	}

}*/


int16_t Engine::minmax(uint8_t depth, int16_t alpha, int16_t beta, uint64_t* zobrist_key)
{
	//check transposition table
	//TODO


	if (depth == 0)
	{
		se.calculate_score(false);
		return se.score;
	}

}