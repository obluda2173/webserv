echo "Content-type: text/html"
echo ""
max=100

random_number=$((RANDOM % max))
echo "<html>"
echo "<head><title>Random Number</title></head>"
echo "<body>"
echo "<h1>Random Number Generator</h1>"
echo "<p>Your random number (0 to $((max - 1))): $random_number</p>"
echo "</body>"
echo "</html>"