echo "Content-type: text/html"
echo ""

# Default values
a=0
b=0
op="add"

# Parse query string: a=5&b=3&op=mul
IFS='&' read -ra PARAMS <<< "$QUERY_STRING"
for param in "${PARAMS[@]}"; do
    key=${param%%=*}
    value=${param#*=}
    case "$key" in
        a) a=$value ;;
        b) b=$value ;;
        op) op=$value ;;
    esac
done

# Perform calculation
result="undefined"
case "$op" in
    add) result=$((a + b)) ;;
    sub) result=$((a - b)) ;;
    mul) result=$((a * b)) ;;
    div)
        if [ "$b" -ne 0 ]; then
            result=$((a / b))
        else
            result="Error: Division by zero"
        fi
        ;;
    *) result="Invalid operation" ;;
esac

# Output
cat <<EOF
<html>
<head><title>CGI Calculator</title></head>
<body>
<h1>CGI Calculator</h1>
<p>Operation: $a $op $b</p>
<p>Result: $result</p>
</body>
</html>
EOF