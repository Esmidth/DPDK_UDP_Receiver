#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <bitset>

typedef std::unordered_map<int, std::vector<int>> int_vector_map;
typedef std::unordered_map<int, std::bitset<16>> int_bitset_map;

int print_map(int_vector_map &map)
{
	printf("--------\n");
	for (auto &n : map)
	{
		// for (auto &t : n.second)
		// {
		// 	printf("key: %d\tValue:%d\n", n.first, t);
		// }
		printf("key: %d\t",n.first);
		for(int i = 0;i<n.second.size();i++)
		{
			printf("Value: %d, ",n.second[i]);
		}
		printf("\n");
	}
}

int print_map(int_bitset_map &map)
{
	printf("--------\n");
	for (auto &n : map)
	{
		std::cout << "key: " << n.first << "\tValue:" << n.second << std::endl;
	}
}

void clear_op()
{
	int_vector_map dhsm_channel_map;

	dhsm_channel_map[1] = {1, 2, 3, 4};

	print_map(dhsm_channel_map);

	dhsm_channel_map.clear();
	printf("----clear\n");

	dhsm_channel_map[2] = {1, 2, 4, 5};
	print_map(dhsm_channel_map);

	int_bitset_map dhsm_channel_map1;
	dhsm_channel_map1[1] = 0x01ff;
	print_map(dhsm_channel_map1);
	printf("count: %d\n", dhsm_channel_map1[1].count());
	for (int i = 0; i < 16; i++)
	{
		printf("%d\n", static_cast<int>(dhsm_channel_map1[1][i]));
		// iterate from low to high
		// for example, indice #0 means the lowest bit of bitset;
	}
}

void modify_op()
{
	// int_vector_map dhsm_channel_map;

	// dhsm_channel_map[1] = {1, 2, 3, 4};
	// print_map(dhsm_channel_map);

	// dhsm_channel_map[1][99] = 99;
	// print_map(dhsm_channel_map);

	int_bitset_map dhsm_channel_map1;
	dhsm_channel_map1[1] = 0x01ff;
	print_map(dhsm_channel_map1);
	printf("count: %d\n", dhsm_channel_map1[1].count());


	dhsm_channel_map1[1][8] = 0;
	print_map(dhsm_channel_map1);
	printf("count: %d\n", dhsm_channel_map1[1].count());

	for (int i = 0; i < 16; i++)
	{
		printf("%d\n", static_cast<int>(dhsm_channel_map1[1][i]));
		// iterate from low to high
		// for example, indice #0 means the lowest bit of bitset;
	}
}

void add_op()
{
	int_vector_map dhsm_channel_map;

	dhsm_channel_map[1] = {1, 2, 3, 4};
	print_map(dhsm_channel_map);

	dhsm_channel_map[1].push_back(77);

	print_map(dhsm_channel_map);
}

void delete_op()
{
	int_vector_map dhsm_channel_map;

	dhsm_channel_map[1] = {1, 2, 3, 4};
	print_map(dhsm_channel_map);

	dhsm_channel_map[1].push_back(77);

	print_map(dhsm_channel_map);
}


void exist_op()
{
	int_bitset_map dhsm_channel_map;

	dhsm_channel_map[1] = 0xfff;
	print_map(dhsm_channel_map);


	if(dhsm_channel_map.find(1) == dhsm_channel_map.end())
	{
		printf("NOT FIND !!\n");
	}else{
		printf("FIND!\n");
	}
}


void test_generate_map2()
{
	int_bitset_map map1;
	int_vector_map map2;

	map1[1] = 0x0001;
	map1[2] = 0x0002;
	map1[3] = 0xff01;


	std::bitset<16> bit_sum = 0x0000;
	for(auto & t:map1)
	{
		bit_sum = bit_sum | t.second;
	}

	print_map(map1);
	std::cout << "bit_sum: " << bit_sum << std::endl;


	for(int i = 0;i<16;i++)
	{
		if(bit_sum[i] == 1)
		{
			map2[i] = {};
		}
	}

	for(auto & item: map1)
	{
		for(int i = 0;i<16;i++)
		{
			if(item.second[i] == 1)
			{
				map2[i].emplace_back(item.first);
			}
		}

	}

	print_map(map2);


}



int main()
{
	test_generate_map2();
}