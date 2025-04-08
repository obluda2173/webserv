**Simple explanation:** A configuration file is a text file where the server’s operational settings are defined in a structured way that your program will read, parse, and use to set itself up properly at startup.

It will **define the following**:
* **host** and **port** to listen on
* what **server name** to use (like example.com)
* how to handle **error pages** (e.g., 404.html)
* **limits** like the maximum request size (to prevent huge, malicious uploads)
* how to **map directories or files** (e.g., serving static files from /var/www)

**Example:**
```
server {
    host 127.0.0.1;
    port 8080;
    server_name myserver.com;

    error_page 404 /errors/404.html;
    client_max_body_size 2M;

    location / {
        root /var/www/html;
        methods GET POST;
    }

    location /api {
        root /var/www/api;
        methods GET;
    }
}
```

