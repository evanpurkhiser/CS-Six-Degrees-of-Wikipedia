#!/usr/bin/env python3

import urllib.request
import urllib.parse
import json
import sys

SEARCH_URL="http://en.wikipedia.org/w/api.php?"

# Use the wiki API to get the article titles
query    = urllib.parse.urlencode({'action': 'opensearch', 'search': sys.argv[1], 'limit': 1})
request  = urllib.request.urlopen(SEARCH_URL + query)
results = json.loads(request.read().decode('utf8'))[1]

# If there was no result exit with error status
if not results:
    exit(1)

print(results[0].replace(' ', '_'))
