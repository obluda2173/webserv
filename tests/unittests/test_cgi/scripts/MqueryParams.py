#!/home/kay/.pyenv/shims python

import os
import urllib.parse

# HTTP header required for CGI
# print("Content-Type: text/html\n")

# Get and parse query string
query_string = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query_string)

for param_name, param_values in params.items():
    values_str = ", ".join(param_values)
    print(f"{param_name} {values_str}")
