#!/bin/bash

EXPECTED_RESPONSE_PATH="../tests/integration_test"

echo "
foo "bar";
server {
  port   8080;
  server_name foo.com;
  root /home/ubuntu/sites/foo/;
}" > example_config_test

./bin/server example_config_test &
pid_server=$!

sleep 1

# Test 1 - Valid Request
printf "Test 1 - Valid Request\n"

test_response=$(printf '%s\r\n%s\r\n%s\r\n\r\n'  \
    "GET / HTTP/1.1"                        \
    "Host: www.example.com"                 \
    "Connection: close"                     \
    | nc 127.0.0.1 8080)
        
echo $test_response > test_response1

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

kill -9 $pid_server
exit 0
