#!/bin/bash

EXPECTED_RESPONSE_PATH="../tests/integration_test"
STATC_FILES_PATH="../static"

echo "
foo "bar";
server {
  port   9080;
  server_name foo.com;
  root /home/ubuntu/sites/foo/;

  endpoint /static STATIC ../static/;
  endpoint /static1 STATIC ../tests/static_files/;
  endpoint /echo ECHO;
  endpoint /echo1 ECHO;
}" > example_config_test

./bin/server example_config_test &
pid_server=$!

sleep 1

# Test 1 - Valid Echo Request
printf "Test 1 - Valid Echo Request\n"

(printf '%s\r\n%s\r\n%s\r\n\r\n' \
    "GET /echo HTTP/1.1"                        \
    "Host: www.example.com"                     \
    "Connection: close"                         \
    | nc localhost 9080) > test_response1

DIFF=$(diff ${EXPECTED_RESPONSE_PATH}/test1_expected test_response1)

if [ "$DIFF" == "" ]; then
    echo "Test 1: Success";  
else 
    echo "Test 1: Failed"; 
    kill -9 $pid_server
    exit 1;
fi

# Test 2 - Invalid Request: Insufficient Request Headers (Missing Port Num):
test_response=$(printf '%s\r\n%s\r\n%s\r\n\r\n'  \
    "GET HTTP/1.1"                          \
    "Host: www.example.com"                 \
    "Connection: close"                     \
    | nc localhost 2>&1) # 2>&1 to output stderr

echo $test_response > test_response2

DIFF=$(diff ${EXPECTED_RESPONSE_PATH}/test2_expected test_response2)

if [ "$DIFF" == "" ]; then
    echo "Test 2: Success";  
else 
    echo "Test 2: Failed"; 
    kill -9 $pid_server
    exit 1;
fi

# Test 3 - Valid Static Request
printf "Test 3 - Valid Static Request\n"

curl http://localhost:9080/static/lorem-ipsum.txt > test_response3

DIFF=$(diff ${STATC_FILES_PATH}/lorem-ipsum.txt test_response3)

if [ "$DIFF" == "" ]; then
    echo "Test 3: Success";  
else 
    echo "Test 3: Failed"; 
    kill -9 $pid_server
    exit 1;
fi

kill -9 $pid_server
exit 0
