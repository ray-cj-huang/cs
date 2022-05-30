cmake_minimum_required(VERSION 3.10.0)
project(pushin-p)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system thread regex log_setup log)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include include/factories include/file_system include/request_handlers)

# Find Bash
find_program (BASH_PROGRAM bash)

# Update name and srcs
add_library(server_lib src/server.cc)
add_library(session_lib src/session.cc)
add_library(config_parser_lib src/config_parser.cc)
add_library(logger src/logger.cc)
add_library(request_handler_lib src/request_handlers/request_handler.cc)
add_library(error_request_handler_lib src/request_handlers/error_request_handler.cc)
add_library(echo_request_handler_lib src/request_handlers/echo_request_handler.cc)
add_library(static_request_handler_lib src/request_handlers/static_request_handler.cc)
add_library(crud_request_handler_lib src/request_handlers/crud_request_handler.cc)
target_link_libraries(crud_request_handler_lib Boost::filesystem)
add_library(health_request_handler_lib src/request_handlers/health_request_handler.cc)
add_library(sleep_request_handler_lib src/request_handlers/sleep_request_handler.cc)
add_library(caption_this_request_handler_lib src/request_handlers/caption_this_request_handler.cc)
target_link_libraries(caption_this_request_handler_lib Boost::filesystem)
add_library(error_request_handler_factory_lib src/factories/error_request_handler_factory.cc)
add_library(echo_request_handler_factory_lib src/factories/echo_request_handler_factory.cc)
add_library(static_request_handler_factory_lib src/factories/static_request_handler_factory.cc)
add_library(crud_request_handler_factory_lib src/factories/crud_request_handler_factory.cc)
add_library(health_request_handler_factory_lib src/factories/health_request_handler_factory.cc)
add_library(sleep_request_handler_factory_lib src/factories/sleep_request_handler_factory.cc)
add_library(caption_this_request_handler_factory_lib src/factories/caption_this_request_handler_factory.cc)

add_library(file_system_base_lib src/file_system/file_system_base.cc)
target_link_libraries(file_system_base_lib Boost::filesystem)
add_library(file_system_real_lib src/file_system/file_system_real.cc)
add_library(file_system_fake_lib src/file_system/file_system_fake.cc)

# Update executable name, srcs, and deps
add_executable(server src/server_main.cc)
target_link_libraries(server server_lib session_lib config_parser_lib
    logger
    file_system_real_lib
    file_system_fake_lib
    file_system_base_lib
    echo_request_handler_factory_lib
    static_request_handler_factory_lib
    error_request_handler_factory_lib
    crud_request_handler_factory_lib
    health_request_handler_factory_lib
    sleep_request_handler_factory_lib
    caption_this_request_handler_factory_lib
    echo_request_handler_lib
    static_request_handler_lib
    error_request_handler_lib
    crud_request_handler_lib
    health_request_handler_lib
    sleep_request_handler_lib
    caption_this_request_handler_lib
    request_handler_lib
    Boost::system Boost::thread Boost::regex
    Boost::log_setup Boost::log)

# Update test executable name, srcs, and deps
add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test server_lib session_lib gtest_main logger error_request_handler_factory_lib
    echo_request_handler_lib static_request_handler_lib error_request_handler_lib request_handler_lib 
    Boost::system Boost::thread Boost::regex Boost::log_setup Boost::log)

add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test session_lib gtest_main logger 
    echo_request_handler_factory_lib static_request_handler_factory_lib error_request_handler_factory_lib 
    echo_request_handler_lib static_request_handler_lib error_request_handler_lib request_handler_lib 
    Boost::system Boost::thread Boost::regex Boost::log_setup Boost::log)

add_executable(request_handler_test tests/request_handler_test.cc)
target_link_libraries(request_handler_test
    echo_request_handler_lib static_request_handler_lib error_request_handler_lib
    crud_request_handler_lib health_request_handler_lib sleep_request_handler_lib
    caption_this_request_handler_lib file_system_fake_lib file_system_base_lib
    request_handler_lib gtest_main logger Boost::system 
    Boost::thread Boost::regex Boost::log_setup Boost::log)

add_executable(request_handler_factory_test tests/request_handler_factory_test.cc)
target_link_libraries(request_handler_factory_test 
    echo_request_handler_factory_lib static_request_handler_factory_lib error_request_handler_factory_lib
    crud_request_handler_factory_lib health_request_handler_factory_lib sleep_request_handler_factory_lib
    caption_this_request_handler_factory_lib
    echo_request_handler_lib static_request_handler_lib error_request_handler_lib crud_request_handler_lib
    health_request_handler_lib sleep_request_handler_lib caption_this_request_handler_lib request_handler_lib
    file_system_fake_lib file_system_base_lib
    gtest_main logger Boost::system Boost::thread Boost::regex 
    Boost::log_setup Boost::log)
    
add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test config_parser_lib gtest_main logger Boost::system
    Boost::thread Boost::regex Boost::log_setup Boost::log)

add_executable(file_system_test tests/file_system_test.cc)
target_link_libraries(file_system_test
    file_system_fake_lib file_system_real_lib file_system_base_lib
    gtest_main logger 
    Boost::system Boost::thread)

# Update with test binary
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_factory_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(file_system_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Add integration test
add_test(NAME IntegrationTest COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_test/test.sh)

# Update with target/test targets
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(
    TARGETS server server_lib session_lib config_parser_lib
        error_request_handler_lib echo_request_handler_lib static_request_handler_lib
        crud_request_handler_lib health_request_handler_lib sleep_request_handler_lib
        caption_this_request_handler_lib request_handler_lib
        error_request_handler_factory_lib echo_request_handler_factory_lib static_request_handler_factory_lib
        crud_request_handler_factory_lib health_request_handler_factory_lib sleep_request_handler_factory_lib
        caption_this_request_handler_factory_lib
        file_system_real_lib file_system_fake_lib file_system_base_lib
    TESTS server_test session_test config_parser_test request_handler_test request_handler_factory_test file_system_test)
