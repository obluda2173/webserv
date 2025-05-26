import cgi

form = cgi.FieldStorage()

# Output headers with correct CRLF and a blank line after headers
print("Content-type: text/html\r")
print("Content-Length: 196\r")
print("\r")  # End of headers

# Now body starts
print("<html><body style='text-align:center;'>")
print("<h1 style='color: green;'>GeeksforGeeks</h1>")

if form.getvalue("name"):
    name = form.getvalue("name")
    print("<h2>Hello, " + name + "!</h2>")
    print("<p>Thank you for using our script.</p>")

if form.getvalue("happy") == "on":
    print("<p>Yayy! We're happy too! 😊</p>")

if form.getvalue("sad") == "on":
    print("<p>Oh no! Why are you sad? 😢</p>")

print("</body></html>")
