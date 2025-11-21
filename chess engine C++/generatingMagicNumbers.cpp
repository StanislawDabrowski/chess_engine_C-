/*
#include <iostream>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <bit>
#include <fstream>
#include <string>
#include <iomanip>
#include <random>
#include <chrono>
#include "MoveGenerator.h"



uint64_t random_magic_candidate()
{
	static std::mt19937_64 rng(
		(uint64_t)std::chrono::high_resolution_clock::now().time_since_epoch().count()
	);
	return rng() & rng() & rng();
}

uint64_t occupancy_from_index_2(uint32_t index, uint64_t mask) {
	uint64_t occ = 0;
	uint32_t bit = 0;
	for (int sq = 0; sq < 64; ++sq) {
		if (mask & (1ULL << sq)) {
			if (index & (1U << bit)) occ |= (1ULL << sq);
			++bit;
		}
	}
	return occ;
}



std::vector<uint64_t> generate_blocker_subsets(MoveGenerator const& mg, int square, bool bishop) {
	std::vector<uint64_t> blockers;
	Bitboard mask = bishop ? mg.bishop_relevant_blockers[square] : mg.rook_relevant_blockers[square];
	int bits = std::popcount(mask);
	uint64_t subset_count = 1ULL << bits;
	for (int i = 0; i < subset_count; i++) {
		blockers.push_back(occupancy_from_index_2(i, mask));
	}
	return blockers;
}


/*Bitboard* compute_attacks_for(int square, bool bishop) {
	return bishop ? mg.bishop_attack_tables[square] : mg.rook_attack_tables[square];
}*//*

uint64_t find_magic(MoveGenerator &mg, int square, int relevant_bits, bool bishop) {
	std::vector<uint64_t> blockers = generate_blocker_subsets(mg, square, bishop);
	Bitboard* attacks = bishop ? mg.bishop_attack_tables[square]
		: mg.rook_attack_tables[square];

	for(int counter = 0;;counter++) {
		uint64_t magic = random_magic_candidate();
		std::unordered_map<uint32_t, Bitboard> used;

		bool fail = false;
		for (int i = 0; i < blockers.size(); i++) {
			uint64_t index = uint64_t((blockers[i] * magic) >> (64 - relevant_bits));
			
			if (used.count(index) && used[index] != attacks[i]) {
				fail = true;
				break;
			}
			used[index] = attacks[i];
		}
		if (!fail) return magic;
		if (counter % 10000 == 0)
			std::cout << "Still searching for square " << square << " (" << (bishop ? "Bishop" : "Rook") << ") after " << counter << " tries\r" << std::flush;
	}
}

int main()
{
	std::ofstream magic_numbers_des_file("magic_numbers_des.txt"); // open file for writing
	std::ofstream magic_numbers_file("magic_numbers.txt"); // open file for writing
	if (!magic_numbers_des_file) {
		std::cerr << "Error opening file\n";
		return 1;
	}
	if (!magic_numbers_file) {
		std::cerr << "Error opening file\n";
		return 1;
	}

	MoveGenerator mg(nullptr);//constructor generates relevant blockers and attack tables


	std::cout << "searching started" << std::endl;
	for (int square = 0; square < 64; square++) {
		int bishop_bits = std::popcount(mg.bishop_relevant_blockers[square]);
		int rook_bits = std::popcount(mg.rook_relevant_blockers[square]);
		uint64_t bishop_magic = find_magic(mg, square, bishop_bits, true);
		uint64_t rook_magic = find_magic(mg, square, rook_bits, false);
		std::cout << "Square: " << square
					<< " Bishop Magic: " << std::hex << bishop_magic
					<< " Rook Magic: " << std::hex << rook_magic << std::dec << std::endl;
		//save magic numbers to file

		magic_numbers_des_file << "Square: " << square
				<< " Bishop Magic: " << std::hex << bishop_magic
			<< " Rook Magic: " << std::hex << rook_magic << std::dec << std::endl;
		magic_numbers_file << std::hex << bishop_magic << "," << rook_magic << std::dec << std::endl;

		//print progress
		std::cout << std::fixed << std::setprecision(2) << "Progress: " << (square + 1) / 64.0f * 100 << "%\r" << std::flush;


	}



	magic_numbers_des_file.close(); // close the file
	magic_numbers_file.close(); // close the file

	return 0;
}*/