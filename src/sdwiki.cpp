#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/time.h>
#include <omp.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <deque>
#include <unordered_map>
#include <algorithm>

// Typedef for our primary data types
typedef std::vector<std::string> pages_t;
typedef std::unordered_map<std::string, int> page_ids_t;
typedef std::unordered_map<int,std::vector<int>> page_links_t;

/* Load in the page titles and page ids from a file.
 *
 * Each line of the file will represent the page ID, starting from line 1 until
 * the end of the file. The page titles will be stored into the pages vector.
 * The page titles => ids will be stored in the page_ids map
 *
 * Returns: The total number of pages loaded
 */
int load_page_titles(const std::string &file_path, pages_t &pages, page_ids_t &page_ids)
{
	std::ifstream page_titles_file(file_path);
	std::string page_name;

	// We won't be using the first index of the vector for anything since
	// the page_id indexes start at 1 instead of 0. This should help to
	// prevent any errors during development
	pages.push_back("");

	// Read the page name into the vector and into the hash-map
	int index = 1;

	while (std::getline(page_titles_file, page_name))
	{
		pages.push_back(page_name);
		page_ids[page_name] = index++;
	}

	return index - 1;
}

/* Load in the page links from a file.
 *
 * Each line of the file should start with a page_id that represents a page, it
 * should then follow with a colon (':'). All numbers after the colon represent
 * page_ids that that page links to.
 *
 * Read all page links into the page_links hash map, where the key is the
 * page_id and the value is a vector of page_ids the page links to.
 *
 * Returns: The total number of links loaded
 */
int load_page_links(const std::string &file_path, page_links_t &page_links)
{
	std::ifstream page_links_file(file_path);
	std::string link_id;

	int current_link = 0;
	int total_links  = 0;

	while (page_links_file >> link_id)
	{
		if (link_id.back() == ':')
		{
			current_link = atoi(link_id.c_str());
		}
		else
		{
			page_links[current_link].push_back(atoi(link_id.c_str()));
			total_links++;
		}
	}

	return total_links;
}

/* Given a hash map containing page_ids that represent a path back to a given
 * root node extract those Ids and order them so that we move from the root node
 * back to the tail_id.
 *
 * The root node is the node whos parent value is -1.
 *
 * Returns: A vector containing the page ids of the path in order.
 */
std::vector<int> path_from_parents(std::unordered_map<int, int> &parent_links,
	int target_id)
{
	int current = target_id;
	std::vector<int> target_path;

	// Only fill the target_path vector if we have a parent link chain
	if (parent_links[target_id])
	{
		// Traverse thrugh the parent_links till we reach the root node
		do
		{
			target_path.push_back(current);
		}
		while ((current = parent_links[current]) != -1);

		// Reverse the target path list to be in proper order
		std::reverse(target_path.begin(), target_path.end());
	}

	return target_path;
}

/* Calculate the shortest path between two page_ids in the page links graph.
 * This uses the Depth First Search algorithm to travers through each of the
 * child nodes looking to see if they are the target id. If none are, then we
 * will traverse through all their children, up until we can find the target id.
 *
 * Returns: See the return value of `path_from_parents`.
 */
std::vector<int> path_between_pages(page_links_t &page_links,
	int start_id, int target_id)
{
	// Keep track of parent nodes so we can traverse back through the path
	std::unordered_map<int, int> parent_links;

	// The starting node will always be considered 'visited'
	parent_links[start_id] = -1;

	// Queue of the current nodes to look at
	std::deque<int> current_nodes;
	current_nodes.push_back(start_id);

	bool found_target = false;

	// Search until we find our target node
	while ( ! found_target && ! current_nodes.empty())
	{
		int available_nodes = current_nodes.size();

		#pragma omp parallel for
		for (int i = 0; i < available_nodes; ++i)
		{
			// Since we can't break out of parallelize loops we just need to
			// finish off what evers left by continuing through them all
			#pragma omp flush (found_target)
			if (found_target) continue;

			int current;

			#pragma omp critical
			{
				current = current_nodes.front();
				current_nodes.pop_front();
			}

			// Iterate through all linked pages
			for (int page_id : page_links[current])
			{
				// Keep track of how we traveled to this node
				#pragma omp critical
				if (parent_links[page_id] == 0)
				{
					parent_links[page_id] = current;

					if (target_id == page_id)
					{
						found_target = true;
						#pragma omp flush (found_target)
					}
					else
					{
						current_nodes.push_back(page_id);
					}
				}
			}
		}
	}

	return path_from_parents(parent_links, target_id);
}

/* Use the external python searcher script to find a given page by a search term
 * and output the resulting page name.
 *
 * Returns: Valid Wikipedia page name, or an empty string if one cannot be found
 */
std::string search_for_page(const std::string &py_searcher_bin, const std::string &search)
{
	FILE *pipe = popen((py_searcher_bin + " \"" + search + "\"").c_str(), "r");

	char buffer[255];
	std::string page_name = "";

	while ( ! feof(pipe))
	{
		if (fgets(buffer, 255, pipe) != NULL)
		{
			page_name += buffer;
		}
	}

	return page_name;
}

int main(int argc, char* argv[])
{
	// Used to store a list of page titles. Each index represents a page title
	pages_t pages;

	// Used to reverse lookup a page ID by it's title
	page_ids_t page_ids;

	// Used to store a list of page_ids that a given page_id links to
	page_links_t page_links;

	std::cout << "Welcome to Six-Degrees-of-Wikipedia path finder\n"
	          << "Please while we load Wikipedia into memory.\n\n";

	// Determine the base directory path
	char path_buffer[1024];
	readlink("/proc/self/exe", path_buffer, sizeof(path_buffer) - 1);
	std::string base_path = std::string(dirname(path_buffer)) + "/../";

	// Load in all page titles into a vector
	std::cout << "\033[92m==>\033[0m Reading in page titles as a vector & map\n";

	{
		struct timeval start, end;

		gettimeofday(&start, 0);
		int total = load_page_titles(base_path + "data/titles-sorted", pages, page_ids);
		gettimeofday(&end, 0);

		double duration = ((end.tv_sec - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
		std::cout << "\033[94m  -> \033[0mRead in " << total    << " page titles\n";
		std::cout << "\033[94m  -> \033[0mTook "    << duration << " seconds\n";
	}

	// Load in all page links into a hash map
	std::cout << "\033[92m==>\033[0m Reading in page links as a map\n";

	{
		struct timeval start, end;

		gettimeofday(&start, 0);
		int total = load_page_links(base_path + "data/links-simple-sorted", page_links);
		gettimeofday(&end, 0);

		double duration = ((end.tv_sec - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
		std::cout << "\033[94m  -> \033[0mRead in " << total    << " page links\n";
		std::cout << "\033[94m  -> \033[0mTook "    << duration << " seconds\n";
	}

	std::cout << "\033[92m==>\033[0m Beginning BFS on Wikipedia graph\n";

	// Get the ID's of the start and target nodes
	int start_id  = page_ids[argv[3]],
	    target_id = page_ids[argv[4]];

	if (start_id == target_id)
	{
		std::cout << "\n\033[92mNo clicks required. The page is the same!\n";
	}

	std::vector<int> target_path;

	struct timeval start, end;

	gettimeofday(&start, 0);
	target_path = path_between_pages(page_links, start_id, target_id);
	gettimeofday(&end, 0);

	double duration = ((end.tv_sec - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
	std::cout << "\033[94m  -> \033[0mTook "    << duration << " seconds\n\n";

	// Print out the path between the pages
	for (int page_id : target_path)
	{
		std::cout << " * " << pages[page_id] << '\n';
	}

	return 0;
}
