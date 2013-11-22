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


## Constructing the graph


## Shortest Path Search


Slides and Code available on GitHub

[github.com/EvanPurkhiser/CS-Six-Degrees-of-Wikipedia](https://github.com/EvanPurkhiser/CS-Six-Degrees-of-Wikipedia)
