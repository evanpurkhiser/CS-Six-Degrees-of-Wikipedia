# Parallel Processing Course Project Proposal

Evan Purkhiser

## Introduction

"Six Degrees of Separation" is a popular hypothesis that states that any two
people in the world are separated by a chain of no more than 6 people. This same
idea can be applied to Wikipedia articles, any two articles are connected
through the articles hyper links with some degree of separation. A [Wikipedia
article](http://en.wikipedia.org/wiki/Wikipedia:Six_degrees_of_Wikipedia)
actually already exists that covers this topic and notes some interesting
results.

Stephen Dolan at Dublin University has tackled this problem as well and offers
some interesting technical insights and will be a good resource during my work
on this project. You can find more information [on his web
page](http://mu.netsoc.ie/wiki/).

For my project I would like to also tackle this problem and create a tool that
could take the entire set of all links between Wikipedia articles, turn this
into a unweighted directed graph, and then perform graph traversal and (perhaps)
various other interesting graph based statistics on the entire graph. Of course,
I would be doing the two most time consuming steps in parallel.

## Parallelizing the graph construction

Wikipedia stores all links between articles as a single table in a SQL
database. Each row in this table specifies a single link from one article to
another article. We can quickly see that this can be represented as a graph
where the nodes are articles and edges are the links between those
articles. Specific details about this table may [be found in the
manual](http://www.mediawiki.org/wiki/Manual:Pagelinks_table). 

Instead of storing this file into a real SQL database, we would instead likely
want to apply some pre-processing to the file to format it into an easily
passable format that we can quickly read by our program. We could then construct
the tree in memory.

Depending on the overhead and thread-safety issues, it may be possible to
parallelize reading in the and creating the tree in memory.

## Parallelizing graph search

The primary goal of this project would be to create a tool that takes in two
article names and determines the shortest path between the two articles using
a depth first search.

Here we would want to parallelize the depth first search algorithm so we can
quickly traverse through the graph to find the shortest path to some node. This
would actually be very similar to what we did in the 3rd lab using Floyd's
algorithm. Though I would like to also investigate any other algorithms that may
perform better on a very large data set like what we will have here.

I've also found that it's been noted that [doing DFS with CUDA is actually a
rather difficult problem](http://stackoverflow.com/a/12671953/790169). So for
this case we will be using OpenMP or pthreads.
