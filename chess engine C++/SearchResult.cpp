#include "SearchResult.h"

SearchResult::SearchResult(int16_t score, Move best_move)
	: score(score), best_move(best_move) { }
SearchResult::SearchResult(int16_t score)
	: score(score) { }