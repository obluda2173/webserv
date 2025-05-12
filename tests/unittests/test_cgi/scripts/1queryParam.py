#!/home/kay/.pyenv/shims python

import os
import urllib.parse

# HTTP header required for CGI
# print("Content-Type: text/html\n")

# Get and parse query string
query_string = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query_string)

# Get parameter 'q' (or empty string if not provided)
query = params.get("q", [""])[0]

# Output HTML
print(f"{query}", end="")
