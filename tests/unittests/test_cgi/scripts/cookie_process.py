#!/usr/bin/env python
# Importing the 'cgi' module
import cgi

# Send an HTTP header indicating the content type as HTML
print("Content-type: text/html\n\n")

# Start an HTML document with center-aligned content
print("<html><body style='text-align:center;'>")

# Display a green heading with text "GeeksforGeeks"
print("<h1 style='color: green;'>You just logged in</h1>")

# Parse form data submitted via the CGI script
form = cgi.FieldStorage()

# Check if the "name" field is present in the form data
if form.getvalue("name"):
    # If present, retrieve the value and display a personalized greeting
    name = form.getvalue("name")

if form.getvalue("password"):
    # If present, retrieve the value and display a personalized greeting
    password = form.getvalue("password")


# Close the HTML document
print("</body></html>")
