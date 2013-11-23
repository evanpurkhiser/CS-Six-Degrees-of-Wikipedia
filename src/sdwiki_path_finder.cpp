#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

int main(int argc, char* argv[])
{
	if (argc != 5)
	{
		std::cerr << "Path Finder only accepts 4 arguments\n";
		exit(1);
	}

	// Load in all page titles into a vector
	std::cout << "\033[92m==>\033[0m Reading in page titles as a vector & map\n";

	std::vector<std::string> pages;
	std::unordered_map<std::string, int> page_ids;

	{
		std::ifstream page_titles_file(argv[1]);
		std::string page_name;

		// We won't be using the first index of the vector for anything since
		// the page_id indexes start at 1 instead of 0. This should help to
		// prevent any errors during devlopment
		pages.push_back("");

		// Read the page name into the vector and into the hash-map
		int index = 1;

		while (std::getline(page_titles_file, page_name))
		{
			pages.push_back(page_name);
			page_ids[page_name] = index++;
		}
		std::cout << "\033[94m  -> \033[0mRead in " << index - 1 << " page titles\n";
	}


	return 0;
}

