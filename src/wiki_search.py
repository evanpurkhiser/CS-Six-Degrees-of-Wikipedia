#!/usr/bin/env python3

import fileinput
import cmd
import urllib.request
import urllib.parse
import json
import os
import subprocess

SEARCH_URL="http://en.wikipedia.org/w/api.php?"

print("Welcome to Six Degress of Wikipedia search!")
print("Enter two wikipedia pages that you would like to find the ")
print("paths shortest paths between")
print("")

# Get start and end search terms
start_search  = input("Start Wikipedia Article: ");
target_search = input("Target Wikipedia Article: ");

# Use the wiki API to get the article titles
search_query  = urllib.parse.urlencode({'action': 'opensearch', 'search': start_search, 'limit': 1})
start_request = urllib.request.urlopen(SEARCH_URL + search_query)
start_title   = json.loads(start_request.read().decode('utf8'))[1][0].replace(' ', '_')

search_query   = urllib.parse.urlencode({'action': 'opensearch', 'search': target_search, 'limit': 1})
target_request = urllib.request.urlopen(SEARCH_URL + search_query)
target_title   = json.loads(target_request.read().decode('utf8'))[1][0].replace(' ', '_')

# Start up the C program to do the search
print("")
print("Searching \033[92m{}\033[0m → \033[94m{}\033[0m\n".format(start_title, target_title))

# Determine the root of the project
root_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..'))

executable = os.path.join(root_dir, 'src', 'sdwiki_path_finder')
data_path  = os.path.join(root_dir, 'data')

# Execute the path finder binary
#
# Arguments are as follows:
#  1. The path to the page-titles data
#  2. The path to the page-links data
#  3. The start wiki article
#  4. The target wiki article
subprocess.call([executable,
         os.path.join(data_path, 'titles-sorted'),
         os.path.join(data_path, 'links-simple-sorted'),
         start_title,
         target_title]);