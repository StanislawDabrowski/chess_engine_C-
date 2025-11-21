#include <iostream>
#include <fstream>
#include <random>



uint64_t random_uint64()
{
	static std::mt19937_64 rng(std::random_device{}());
	static std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
	return dist(rng);
}



int main4()
{
	const int num_values = 12 * 64 + 4 + 16;
	uint64_t zobrist_values[num_values];
	uint64_t temp;
	for (int i = 0; i < num_values;i++)
	{
		temp = random_uint64();
		//check if temp is already in the array
		bool found = false;
		for (int j = 0; j < i; j++)
		{
			if (zobrist_values[j] == temp)
			{
				found = true;
				break;
			}
		}
		if (found)
		{
			i--;
			continue;
		}
		zobrist_values[i] = temp;
	}

	//write to file
	std::ofstream file("zobrist_values.txt");
	if (file.is_open())
	{
		for (int i = 0; i < num_values; i++)
		{
			file << "0x" << std::hex << zobrist_values[i] << (i == num_values - 1 ? "" : "\n");
		}
		file.close();
	}
	else
	{
		std::cout << "Unable to open file";
	}


	return 0;
}