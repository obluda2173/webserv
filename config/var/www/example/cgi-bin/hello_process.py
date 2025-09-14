#!/usr/bin/env python
# Importing the 'cgi' module
import cgi

# Send an HTTP header indicating the content type as HTML
print("Content-type: text/html\n\n")

# Start an HTML document with center-aligned content
print("<html><body style='text-align:center;'>")

# Display a green heading with text "GeeksforGeeks"
print("<h1 style='color: green;'>Webserv</h1>")

# Parse form data submitted via the CGI script
form = cgi.FieldStorage()

# Check if the "name" field is present in the form data
if form.getvalue("name"):
    # If present, retrieve the value and display a personalized greeting
    name = form.getvalue("name")
    print("<h2>Hello, " + name + "!</h2>")
    print("<p>Thank you for using our script.</p>")

# Check if the "happy" checkbox is selected
if form.getvalue("happy") == "on":
    # If selected, display a message with a happy emoji
    print("<p>Yayy! We're happy too!</p>")

# Check if the "sad" checkbox is selected
if form.getvalue("sad") == "on":
    # If selected, display a message with a sad emoji
    print("<p>Oh no! Why are you sad?</p>")

# Close the HTML document
print("</body></html>")