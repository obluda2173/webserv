#!/usr/bin/env bash

curl --location --request POST 'http://localhost:8082/post_body' \
--header 'Host: portfolio.com'

curl --location 'http://localhost:8082/post_body' \
--header 'Host: example.com''

curl --location 'http://localhost:8082/post_body' \
--header 'Host: portfolio.com'

curl --location 'http://localhost:8082' \
--header 'Host: portfolio.com'

curl --location 'http://localhost:8082' \
--header 'Host: example.com'

curl --location --request POST 'http://localhost:8082/post_body' \
--header 'Host: portfolio.com'
