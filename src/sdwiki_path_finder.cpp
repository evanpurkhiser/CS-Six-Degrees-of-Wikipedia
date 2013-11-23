#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <ctime>

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
		std::clock_t start = std::clock();

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

		double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
		std::cout << "\033[94m  -> \033[0mtook " << duration << " seconds\n";
	}

	// Load in all page links into a hash map
	std::cout << "\033[92m==>\033[0m Reading in page links as a map\n";

	std::unordered_map<int,std::vector<int>> page_links;

	{
		std::clock_t start = std::clock();

		std::ifstream page_links_file(argv[2]);
		std::string link_id;

		int current_link = 0;

		while (page_links_file >> link_id)
		{
			if (link_id.back() == ':')
			{
				current_link = atoi(link_id.c_str());
			}
			else
			{
				page_links[current_link].push_back(atoi(link_id.c_str()));
			}
		}

		double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
		std::cout << "\033[94m  -> \033[0mtook " << duration << " seconds\n";
	}

	int page_id = page_ids[argv[3]];
	std::cout << argv[3] << "(" << page_id << ") links to:\n";

	for (int i : page_links[page_id])
	{
		std::cout << " * " << pages[i] << std::endl;
	}

	return 0;
}

