#!/bin/bash

EXPECTED_RESPONSE_PATH="../tests/integration_test"
STATC_FILES_PATH="../static"

echo "
foo "bar";
server {
  port   9080;
  server_name foo.com;
  root /home/ubuntu/sites/foo/;

  location /static StaticHandler {
    root ../static/;
  }

  location /static1 StaticHandler {
    root ../tests/static_files/; # supports relative paths
  }

  location /echo EchoHandler { # no arguments
  }

  location /echo1 EchoHandler {
  }

  location /api CRUDHandler {
    root ./crud/;
  }

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


# Acknowledgement: reference code: 
# https://code.cs130.org/plugins/gitiles/buugle/+/8b8b0ff3ed34b7401f2235df8e5c3cfb7196b8a4/tests/integration_test.sh
# CRUD tests
mkdir crud

# Create Test 1
echo -ne "{\"id\":1}" > crud_test_sample_result
curl -X POST http://localhost:9080/api/objects -d "{\"name\": \"object 1\"}" > crud_test_actual_result

crud_test_diff=$(diff -w crud_test_actual_result crud_test_sample_result)
if [ -z "$crud_test_diff" ]
then
    echo "PASSED Integration Test for Create Test 1"
else
    echo "Compared actual:"
    cat crud_test_actual_result 1>&2
    echo
    echo "with sample:"
    cat crud_test_sample_result 1>&2
    exit_code=1
fi

# Create Test 2
echo -ne "{\"id\":2}" > crud_test_sample_result
curl -X POST http://localhost:9080/api/objects -d "{\"name\": \"object 2\"}" > crud_test_actual_result

crud_test_diff=$(diff -w crud_test_actual_result crud_test_sample_result)
if [ -z "$crud_test_diff" ]
then
    echo "PASSED Integration Test for Create Test 2"
else
    echo "Compared actual:"
    cat crud_test_actual_result 1>&2
    echo
    echo "with sample:"
    cat crud_test_sample_result 1>&2
    exit_code=1
fi

# Retrieve Test 1
echo -ne "{\"name\": \"object 1\"}" > crud_test_sample_result
curl -X GET http://localhost:9080/api/objects/1 > crud_test_actual_result

crud_test_diff=$(diff -w crud_test_actual_result crud_test_sample_result)
if [ -z "$crud_test_diff" ]
then
    echo "PASSED Integration Test for Retrieve Test 1"
else
    echo "Compared actual:"
    cat crud_test_actual_result 1>&2
    echo
    echo "with sample:"
    cat crud_test_sample_result 1>&2
    exit_code=1
fi

# Retrieve Test 2
echo -ne "{\"name\": \"object 2\"}" > crud_test_sample_result
curl -X GET http://localhost:9080/api/objects/2 > crud_test_actual_result

crud_test_diff=$(diff -w crud_test_actual_result crud_test_sample_result)
if [ -z "$crud_test_diff" ]
then
    echo "PASSED Integration Test for Retrieve Test 2"
else
    echo "Compared actual:"
    cat crud_test_actual_result 1>&2
    echo
    echo "with sample:"
    cat crud_test_sample_result 1>&2
    exit_code=1
fi

# Update Test 1
echo -ne "{\"name\": \"updated object\"}" > crud_test_sample_result
curl -X PUT http://localhost:9080/api/objects/2 -d "{\"name\": \"updated object\"}"
curl -X GET http://localhost:9080/api/objects/2 > crud_test_actual_result

crud_test_diff=$(diff -w crud_test_actual_result crud_test_sample_result)
if [ -z "$crud_test_diff" ]
then
    echo "PASSED Integration Test for Update Test 1"
else
    echo "Compared actual:"
    cat crud_test_actual_result 1>&2
    echo
    echo "with sample:"
    cat crud_test_sample_result 1>&2
    exit_code=1
fi

# List Test 1
echo -ne "[1, 2]" > crud_test_sample_result
curl -X GET http://localhost:9080/api/objects/ > crud_test_actual_result

crud_test_diff=$(diff -w crud_test_actual_result crud_test_sample_result)
if [ -z "$crud_test_diff" ]
then
    echo "PASSED Integration Test for List Test 1"
else
    echo "Compared actual:"
    cat crud_test_actual_result 1>&2
    echo
    echo "with sample:"
    cat crud_test_sample_result 1>&2
    exit_code=1
fi

# Delete Test 1
echo -ne "[2]" > crud_test_sample_result
curl -X DELETE http://localhost:9080/api/objects/1
curl -X GET http://localhost:9080/api/objects/ > crud_test_actual_result

crud_test_diff=$(diff -w crud_test_actual_result crud_test_sample_result)
if [ -z "$crud_test_diff" ]
then
    echo "PASSED Integration Test for Delete Test 1"
else
    echo "Compared actual:"
    cat crud_test_actual_result 1>&2
    echo
    echo "with sample:"
    cat crud_test_sample_result 1>&2
    exit_code=1
fi

rm -rf crud
rm crud_test_sample_result
rm crud_test_actual_result

kill -9 $pid_server
exit 0