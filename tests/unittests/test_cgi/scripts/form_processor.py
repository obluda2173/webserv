#!/usr/bin/env python3
import cgi, cgitb 

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
name = form.getvalue('name')
hobby  = form.getvalue('hobby')

print("Content-type:text/html\r\n")
print("name " + name)
print("hobby " + hobby)
