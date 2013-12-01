# Six Degrees of Wikipedia

Evan Purkhiser

## Introduction

The concept of "Six Degrees of Separation" is that any two given entities are no
more than six steps away from each other in _some sense_. Many different
examples of this exist, one of the more popular examples is the "Kevin Bacon"
game, the idea being that any actor may be linked to Kevin Bacon through less
than six movies.

This same idea can be applied to Wikipedia, thus the name "Six Degrees of
Wikipedia". In this case, the idea is that any two Wikipedia articles are no
more than six articles apart, where the connecting factor is the hyper links
between the different articles. We can quickly see that this idea forms a very
common data structure: A directed unweighted graph. The goal of the project is
to construct this graph from a given set of Wikipedia data and then preform a
shortest path algorithm on the graph to find the number of clicks between two
articles.

## The Methodology

The process of going from zero to a working utility allowing a user to enter in
two articles and find the distance between them went as follows

 1. Collect data from Wikipedia that links articles together.

 2. Transform the data into a easily parsable and machine-readable format used
	to construct the directed unweighted graph of articles. This includes a
	data file containing a list of article titles and article IDs, as well as a
	data file including article ID's and the article ID's they link to.

 3. Construct a program in C++11 that will use the collected data to allow the
    user to find the distance between two articles. A high level overview of how
    this works is as follows:

	1. Read the article titles into a vector and into a hash map. Allowing us to
	   lookup a article title from a article ID, as well as look up a article ID
	   from a article title.

	2. Read in the article links into a map. This map will form our graph data
	   structure in that each element will contain a vector of page IDs. That
	   vector may be accessed in $O(1)$ time (best case for a map) where the key
	   is the article ID that contains those page links. This is the most
	   important data construct in the program.

	3. Prompt the user for two Wikipedia articles to search for the shortest
	   path between. Because we want the interactive program to be natural and
	   intutive to use we will use a external script using the Wikipedia API to
	   lookup the "real" Wikipedia page title from the two given queries.

	4. Use the Breadth First Search (BFS) algorithm to traverse from the start
	   node through the graph until we can locate the target node. Here it's
	   important to keep track of the parent nodes during the search to ensure
	   we can iterate the path once the target is located.

	5. Output the path and return to step 3.

We will be using OpenMP as the parallelization framework throughout the C++11
program to provide us with relatively quick and easy parallelization.

### Collecting data from Wikipedia

Wikipedia offers a host of different data sets that are collected yearly,
monthly, and for some data sets, even daily. All for free use. The primary
reason for this is to allow anyone to host a local mirror of Wikipedia. For our
purposes however we are most interested in a data set that will provide us with
the links between different pages.

We can use the Wikipedia daily MySQL database dumps of the `pagelinks` table and
the `pages` table to collect precisely the data we need.

After extracting the database dumps from the downloaded Gzipped files we had two
files that totaled about 32gb total.

 * `pagelinks`: 30Gb
 * `pages`: 2.6Gb

### Transforming the collected data

Both downloaded files are in the MySQL database dump format. This includes a lot
of text that we don't care about, and it was hopped that we could strip down the
`pagelinks` and `pages` file down to more manageable file sizes.

The goal was to have two small, data-only, files that could be very easily read
in by the C++ program.

----

The first table looked at processing was the `pages` table. For this table the
goal is to transform the database insertions into a file that would match the
following format:

    3523523 This_is_a_Article_Title
    3523524 Another_Article_Title
    ...

Where the first number in the line is the ID of that given page followed by a
space then the Wikipedia article title (which is gaunted to have no spaces as
described in the [Wikipedia
manual](http://www.mediawiki.org/wiki/Manual:Page_title)).

Since the `pages` database dump contains multiple SQL `INSERT INTO` statements,
each containing multiple row inserts would need to be parsed out this data using
more complicated logic than just looking at each line in the file as a single
page entry. It was decided that the quickest and easiest way to do this would be
to write a small python script to parse the `INSERT` statements and write the
`page_id` and `page_title` columns out to a file in the format described above.

The written script took about 30 seconds to execute and produced a file that was
about 300Mb large, down from the 2.6Gb database dump that we stared with.

----

The `pagelinks` table was a whole other beast. This file is about 30 gigabytes
worth of unzipped data. Not only that, but it was also quite a bit off from the
format that was hopped for.

Again, since we will be reading the file in with our program we will want to
transform the data to an optimal format. The goal format is as follows:

    3523523 643 235981 39634 128385
    3523524 1221 64343 4523 199243
    ...

Where the first number represents the article ID and where the following numbers
are article IDs that that particular page has links too. Unfortunately the
Wikipedia database dump only contains the first article ID, the links are then
identified by the article titles.

In order to extract the data in the format described above, some extra logic was
going to be required to transform the linked article title into a article ID.
Again it was decided that the simplest way to extract the data would be to use a
python script. The initial plan was to load in the previously generated pages
file into a map that could be used to lookup the article ID from a article
title. Each article Id would then keep track of a list of article IDs they link
to, building upon this list as we parse through the SQL file.

However, after spending over a days worth of work attempting to follow through
and parse out the data it was concluded that it would be far easier to use a
data set that was already pre processed to be in the required format.

### A new data set

Henry Haselgrove, A PHD student at the University of Queensland, provides a
[Wikipedia page links dataset](http://haselgrove.id.au/wikipedia.htm) that
provides the necessary data in the precise format that we were looking for. The
`titles-sorted` file contains a list of Wikipedia page titles where each line
simply contains the article title and the line number represents the article ID.
The `links-simple-sorted` file is very close to the format described previously
that we were hoping to transform the data into. The only minor difference is that
a colon (`:`) was placed just after the first article ID.

The only downside of this data set is that it is from early 2009.

### Parallelizing the data loading

During the initial implementation of the `sdwiki` client data loading took about
30 seconds. The primary slow down was due to the parsing and vector generation
of the page links data set. Loading in the article titles themselves was
actually rather in-expensive and for the most part was IO bound.

Instead of processing each line of the page links as we read it in, we can
actually offload this into separate threads. We do this by first reading the
entire file, line by line, into memory. We can then iterate over every one of
these lines and perform the grunt work in parallel using OpenMP.

By parallelizing this portion of the `sdwiki` client it was it was possible to
speed up the "boot" time of the program as a whole by a factor of three.

### Using Breadth First Search

After loading in all data, the primary goal of the `sdwiki` client is to search
through the graph of Wikipedia articles to find a given target article from some
starting article. Because the graph is unweighted and directed the most obvious
choice to search through the tree is to use the BFS algorithm. Before choosing
to use this algorithm research was done on various other graph search
algorithms. However because the graph is so simple, a complex algorithm to path
find between two nodes was not required.

Due to the way BFS works, it's actually possible to parallelize a portion of the
search. First, reviewing the BFS algorithm (as [described on
Wikipedia](http://en.wikipedia.org/wiki/Breadth-first_search))

 1. Enqueue the root node
 2. Dequeue a node and examine it
    * If the element sought is found in this node, quit the search and return a
        result.
    * Otherwise enqueue any successors (the direct child nodes) that have not yet been
      discovered.
 3. If the queue is empty, every node on the graph has been examined – quit the
    search and return "not found".
 4. If the queue is not empty, repeat from Step 2.

This algorithm will run in $O(|V|)$ time, where $V$ is the number of vertices, or
total pages.

Since we will be queuing up nodes to be inspected (to check if they are the
target) we can see that it would actually be possible to inspect the nodes in
parallel instead of in an iterative manor.

Unfortunately though, because we must check if the target is already found
before performing the node inspection and queuing the child nodes, the whole
loop essentially becomes a critical section. In theory, parallelizing the search
would give us a time complexity of $O(|V|/p)$ where $p$ is the number of
processors. However, because of the critical section, the number of processor
doesn't matter as each iteration must happen in serial anyway. We've also now
gained some overhead due to the thread startup and critical section handling,
thus causing the overall algorithm to actually **be slower** when parallelized.

# Test Cases

Due to the nature of the problem this project looks at, we aren't actually
running an algorithm against different sets of data. It would be possible to
look at every combination of pages and do some interesting analysis on this,
however due to time constraints this would be rather difficult.

Over the development of the program an interesting characteristic was noticed
of certain target articles. Some would take a rather long time to find the
shortest path. One example of this is 'just-about-anything' to 'GitHub'. This
particular query takes about 19 seconds to complete.

It is  suspected that the reason this case takes so long is due to the fact
that the Wikipedia page links data set is from early 2009. The company GitHub
was founded in mid 2008, so at the time it was a relatively small company. What
this means is that the GitHub article in 2009 probably had relatively few
'in-links' (links from other pages).

I've confirmed this by looking up the GitHub ID from the `titles-sorted` file
and then grepping through the `links-simple-sorted` file for that ID.

    $ grep -c 1971182 links-simple-sorted
    3

Ignoring the GitHub article itself we can see that only two other articles link
to the GitHub page. From this it can be reasoned that articles with few in-links
will in general take longer to search for in the graph.

For most other articles the search will take just microseconds.

\pagebreak

# Discussion

During the development of the `sdwiki` client research was done looking into
different algorithms that would be a best fit for searching a directed
unweighted graph to find the shortest path between two nodes. Some techniques
were found that are able to do optimized BFS against a large dataset using
tools such as
[hadoop](http://www.johnandcailin.com/blog/cailin/breadth-first-graph-search-using-iterative-map-reduce-algorithm)
however no algorithms other than BFS were found that could be efficiently used
to search a graph in parallel.

## Future Work and Comments

 * It would be nice to go back and focus on Wikipedia data acquisition and write
   an efficient tool to extract the page links and page titles into the formats
   used for the `sdwiki` tool. Instead of using Python to do the parsing for
   large files it would be far better to use C++ or even C which would have a
   much easier time handling the large files.

 * Perhaps a more optimal file format could be used for storing the page links
   other than the format described previously. Having full control over this
   could allow us to optimize the way the data is loaded into memory. Perhaps
   even a way to load the file directly into memory as a serialized object.

 * More time could have been spent on parallelizing the breadth first search
   algorithm. It's suspected that there could be some tricks or variations of
   the algorithm that could be used to actually improve performance when run in
   parallel.

Overall I found this project to be very fun and interesting to work on. I'm
glad I picked a problem that I could relate to instead of simply implementing a
algorithm that would be run on various sets of data.

# Conclusion

The goal of this project was to create a tool that would allow someone to enter
two Wikipedia articles and find the shortest click-path between those two
articles. The driving force behind the project being that we could parallelize
both the loading of data as well as the searching of the graph of articles.

In the end we were able to very successful parallelize the loading of data into
memory. However, parallelizing the BFS algorithm ended in failure. While the
algorithm certainly did work in parallel, it turned out to be slower due to the
OpenMP overhead.
