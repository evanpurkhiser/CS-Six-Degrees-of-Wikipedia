# Six Degrees of Wikipedia
## By Evan Purkhiser


## Six Degrees of Separation


A theory that everyone and everything is six or less steps away. For example,
the idea of "A friend of a friend".


### Applying this to Wikipedia..

Given any two Wikipedia pages, how many clicks does it take to get from one to
the other.


# Gathering Data


Wikipedia [offers database
downloads](http://en.wikipedia.org/wiki/Wikipedia:Database_download) of various
different tables as SQL dumps. The two tables we are interested in are the
`page_links` and `pages` tables.


### `page_links`

 * `page_id`, `link_namespace`, `link_page_title`
 * Would be all we need, however the link is to the text title instead of the
   `page_id` like we would prefer
 * Gzipped file is 5.1Gb
 * Extracted the SQL dump is **29.5Gb**


### `pages`

 * Contains various meta-data about Wikipedia pages
 * Includes a `page_namespace`. Name space `0` includes all 'real' wikipedia
   articles (not Talk pages, Discussion, etc).
 * Also includes `page_id` and `page_title`
 * Gzipped file is only 0.9Gb
 * Extracted the SQL dump is about 2.6Gb


## What we want

 * A file with just page IDs and titles
 * A file with a single page ID and a list of page IDs it links too


## Processing the Data

 * Extract out page IDs and page titles into a single file.
   * Use a python script to parse the SQL `INSERT`'s 
   * Write each line as `page_id,page_title`
   * Took about 30 seconds. File was 320Mb
 * Extract page links with only IDs
   * Read in generated `page_id,page_title` file as map
   * Store page IDs in a map until we've read all links in
   * Write out each page_id's links to a file
   * _But..._


## Big Data is _hard_

 * Spent about a day trying to extract data
 * Took a long time to process data
 * Not really the focus of my project
 * Someone has to have done this already right?


## Henry Haselgrove

Provides a Wikipedia dataset of page-links and pages-titles in the very exact
format that I was looking for.

 * page-links file is only about 1Gb
 * page-titles file is only 107Mb
 * Excludes all uninteresting pages (Talk, Discussion, etc)
 * [Available for download from his
   webpage](http://haselgrove.id.au/wikipedia.ht)


## The downside is...

This data is from early 2009. That's almost 5 years old.


![Wikipedia
growth](http://upload.wikimedia.org/wikipedia/commons/2/26/EnwikipediaArt.PNG)


## Better than nothing

At this point I had already spent too much time extracting data. Better to just
use it than keep trying to get current data in the format I want.


# The program


 * Written in C++11
 * Uses OpenMP for parallizing sections of code
 * Uses an external python script to search Wikipedia page names from a seach
   term
 * Interactive console type application


## Loading in the Data


### Step 1: Reading page titles

 * The line number represents the `page_id`
 * Read all titles into a `std::vector<std::string>`
 * Took about 5 seconds
 * Was able to optimize this by reserving ~6 million elements
 * Now it takes ~3 seconds
 * Not paralizable


```cpp
std::ifstream page_titles_file(file_path);
std::string page_name;

int index = 1;

while (std::getline(page_titles_file, page_name))
{
	pages.push_back(page_name);
	page_ids.insert({page_name, index++});
}
```


## Step 2: Reading in page links

 * Each line represents a page and it's links
 * The first number is the `page_id` followed by a "`:`"
 * All numbers after are `page_id`s it links too
 * The link graph is stored into a  `std::unordered_map<int, std::vector<int>>`
 * Original loading logic took ~28 seconds


### We can do better...

 * Read all lines into a `std::vector<std::string>`
 * `#pragma omp parallel for`
 * Do work to extract the `page_id` and create a vector of all `page_id`s it
   links to
 * Enforce critical section on hash map inserts
 * Much faster. Takes ~9 seconds utilizing 8 cores


```cpp
#pragma omp parallel for reduction(+:total_links)
for (int i = 0; i < lines_to_parse; ++i)
{
	std::string line = intermediate[i];
	int page_id = atoi(line.c_str());

	// Remove the page_id, leaving only the links
	line.erase(0, line.find(':') + 1);

	std::istringstream line_stream(line);

	// Construct array from list of page links
	std::vector<int> links((std::istream_iterator<int>(line_stream)),
			std::istream_iterator<int>());

	total_links += links.size();

	#pragma omp critical
	page_links.insert({page_id, links});
}
```


### Total Boot Time: 12 seconds


## Constructing the graph


## Shortest Path Search


Slides and Code available on GitHub

[github.com/EvanPurkhiser/CS-Six-Degrees-of-Wikipedia](https://github.com/EvanPurkhiser/CS-Six-Degrees-of-Wikipedia)
