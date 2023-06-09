#!/bin/bash

EXPECTED_RESPONSE_PATH="../tests/integration_test"
STATIC_FILES_PATH="../static"

rm example_config_test

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

  location /sleep SleepHandler {
  }

  location /api CRUDHandler {
    root ./crud/;
  }

  location /health HealthHandler {
  }

  location /caption CaptionThisHandler {
      root ../tests/captions;
  }
}" > example_config_test

./bin/server example_config_test &
pid_server=$!

sleep 1

rm example_config_test

# Test 1 - Valid Echo Request
printf "Test 1 - Valid Echo Request\n"

rm test_response1

(printf '%s\r\n%s\r\n%s\r\n\r\n' \
    "GET /echo HTTP/1.1"                        \
    "Host: www.example.com"                     \
    "Connection: close"                         \
    | nc localhost 9080) > test_response1

DIFF=$(diff ${EXPECTED_RESPONSE_PATH}/test1_expected test_response1)

rm test_response1

if [ "$DIFF" == "" ]; then
    echo "Test 1: Success";  
else 
    echo "Test 1: Failed"; 
    kill -9 $pid_server
    exit 1;
fi

# Test 2 - Invalid Request: Insufficient Request Headers (Missing Port Num):
rm test_response2

test_response=$(printf '%s\r\n%s\r\n%s\r\n\r\n'  \
    "GET HTTP/1.1"                          \
    "Host: www.example.com"                 \
    "Connection: close"                     \
    | nc localhost 2>&1) # 2>&1 to output stderr

echo $test_response > test_response2

DIFF=$(diff ${EXPECTED_RESPONSE_PATH}/test2_expected test_response2)

rm test_response2

if [ "$DIFF" == "" ]; then
    echo "Test 2: Success";  
else 
    echo "Test 2: Failed"; 
    kill -9 $pid_server
    exit 1;
fi

# Test 3 - Valid Static Request
printf "Test 3 - Valid Static Request\n"

rm test_response3

curl http://localhost:9080/static/lorem-ipsum.txt > test_response3

DIFF=$(diff ${STATIC_FILES_PATH}/lorem-ipsum.txt test_response3)

rm test_response3

if [ "$DIFF" == "" ]; then
    echo "Test 3: Success";  
else 
    echo "Test 3: Failed"; 
    kill -9 $pid_server
    exit 1;
fi

# Test 4 - Health Request
printf "Test 4 - Valid Health Request\n"

OUT=$(curl http://localhost:9080/health)

if [ $OUT == "OK" ]; then
    echo "Test 4: Success";  
else 
    echo "Test 4: Failed"; 
    kill -9 $pid_server
    exit 1;
fi

# Test 5 - Multithreaded Requests
printf "Test 5 - Valid Multithreaded Requests\n"

OUT=$((curl -s http://localhost:9080/sleep > /dev/null) & time -p (curl -s http://localhost:9080/ > /dev/null) 2>&1)

time_regex="real ([0-9]+\.[0-9]+)"

time_limit="5.00"

if [[ $OUT =~ $time_regex ]]
then
    TIME="${BASH_REMATCH[1]}"
    echo "Request took $TIME seconds"
    if [[ $(echo "$TIME $time_limit" | awk '{print ($1 < $2)}') == 1 ]]
    then      
        echo "Test 4: Success";  
    else
        echo "Test 4: Failed, request response time exceeded time limit of $time_limit"; 
        kill -9 $pid_server
        exit 1;
    fi
fi

# Test 6 - 400 Bad Request
printf "Test 6 - 400 Bad Request\n"

(echo 'hi' | nc localhost 9080) > test_response6

DIFF=$(diff ${STATIC_FILES_PATH}/400_error.html test_response6)
EXPECTED=$(cat ${EXPECTED_RESPONSE_PATH}/test6_expected)

rm test_response6

if [ "$DIFF" == "$EXPECTED" ]; then
    echo "Test 6: Success";  
else 
    echo "diff $DIFF"
    echo "expected $EXPECTED"
    echo "Test 6: Failed"; 
    kill -9 $pid_server
    exit 1;
fi

# Test 7 - 404 File Not Found
printf "Test 6 - 404 File Not Found\n"

(curl http://localhost:9080/static/notexist.dne) > test_response7

DIFF=$(diff ${STATIC_FILES_PATH}/404_error.html test_response7)

rm test_response7

if [ "$DIFF" == "" ]; then
    echo "Test 7: Success";  
else 
    echo "Test 7: Failed"; 
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
    echo "PASSED Integration Test for CRUD Create Test 1"
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
    echo "PASSED Integration Test for CRUD Create Test 2"
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
    echo "PASSED Integration Test for CRUD Retrieve Test 1"
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
    echo "PASSED Integration Test for CRUD Retrieve Test 2"
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
    echo "PASSED Integration Test for CRUD Update Test 1"
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
    echo "PASSED Integration Test for CRUD List Test 1"
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
    echo "PASSED Integration Test for CRUD Delete Test 1"
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

# Caption This! Tests
printf "\nRUNNING CAPTION THIS HANDLER TESTS\n\n"
rm -rf ../tests/captions
rm ct_test_sample_result
rm ct_test_actual_result

# Create Test
echo -ne "{\"id\":1}" > ct_test_sample_result
printf "foo\nbar\nhttps://test.com/xyz.jpg" | curl -X POST http://localhost:9080/caption/submit -d "$(</dev/stdin)" > ct_test_actual_result

ct_test_diff=$(diff -w ct_test_actual_result ct_test_sample_result)
if [ -z "$ct_test_diff" ]
then
    echo "PASSED Integration Test for CT Create Test (matching response)"
else
    echo "Compared actual:"
    cat ct_test_actual_result 1>&2
    echo
    echo "with sample:"
    cat ct_test_sample_result 1>&2
    exit_code=1
    kill -9 $pid_server
    exit 1;
fi

echo -ne "foo\nbar\nhttps://test.com/xyz.jpg" > ct_test_sample_result
cat ../tests/captions/submit/1 > ct_test_actual_result

ct_test_diff=$(diff -w ct_test_actual_result ct_test_sample_result)
if [ -z "$ct_test_diff" ]
then
    echo "PASSED Integration Test for CT Create Test (file generated)"
else
    echo "Compared actual:"
    cat ct_test_actual_result 1>&2
    echo
    echo "with sample:"
    cat ct_test_sample_result 1>&2
    exit_code=1
    kill -9 $pid_server
    exit 1;
fi

# Retrieve Test
curl -X GET http://localhost:9080/caption/1 > ct_test_actual_result

ct_test_diff=$(diff ${EXPECTED_RESPONSE_PATH}/ct_retrieve_expected ct_test_actual_result)
if [ "$ct_test_diff" == "" ]
then
    echo "PASSED Integration Test for CRUD Retrieve Test 1"
else
    echo "Compared actual:"
    cat ct_test_actual_result 1>&2
    echo
    echo "with sample:"
    cat "${EXPECTED_RESPONSE_PATH}/ct_retrieve_expected" 1>&2
    exit_code=1
    kill -9 $pid_server
    exit 1;
fi

# TODO(bryanjwong): Add gallery tests, and additional tests for other functionality

rm -rf ../tests/captions
rm ct_test_sample_result
rm ct_test_actual_result

# Load Test (25 requests/second)
printf "Running Load Test."

# Desired rate is NUM_ITERS/SECS_ALLOTTED requests/sec
NUM_ITERS=100;
SECS_ALLOTTED=10;

echo $NUM_ITERS $SECS_ALLOTTED $RATE

rm ct_test_temp
for i in $(seq 1 1 $NUM_ITERS); do
    printf "iter $i\nfoo\nbar" | curl -X POST http://localhost:9080/caption/submit -d "$(</dev/stdin)" >> ct_test_temp &
done;
sleep $SECS_ALLOTTED
cp ct_test_temp ct_test_actual_result
COUNT=`grep -ow '{ "id":' ct_test_actual_result | wc -l`
if [ $COUNT -lt $NUM_ITERS ]
then
    echo "FAIL: Only $COUNT requests completed in $SECS_ALLOTED seconds (below $NUM_ITERS/$SECS_ALLOTTED requests/sec)."
    kill -9 $pid_server
    exit 1;
fi

rm -rf ../tests/captions
rm ct_test_temp
rm ct_test_actual_result

printf "All tests succeeded!"
kill -9 $pid_server
exit 0
