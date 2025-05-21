#!/usr/bin/env python3
import os
import sys
from urllib.parse import parse_qs

def main():
    # Get content length from environment
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    
    # Read POST data from stdin
    post_data = sys.stdin.read(content_length)
    
    # Parse URL-encoded form data
    parsed_data = parse_qs(post_data)
    
    # Prepare response
    print("Content-Type: text/plain")
    print()
    
    # Format output as specified in test case
    for key in sorted(parsed_data.keys()):
        values = parsed_data[key]
        # Join multiple values with comma (handles arrays)
        print(f"{key}: {', '.join(values)}")

if __name__ == "__main__":
    main()