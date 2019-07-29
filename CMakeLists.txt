cmake_minimum_required(VERSION 3.13)
project(fsdb CXX)

if("${TARGET_CPU}" STREQUAL "")
	set(TARGET_CPU ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif()

string(TOLOWER "${TARGET_CPU}" TARGET_CPU)

if("${TARGET_CPU}" STREQUAL "amd64" OR "${TARGET_CPU}" STREQUAL "x86_64")
	set(CMAKE_SIZEOF_VOID_P 8)
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB32_PATHS FALSE)
elseif("${TARGET_CPU}" STREQUAL "x86")
	set(CMAKE_SIZEOF_VOID_P 4)

	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS FALSE)
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB32_PATHS TRUE)

	if(GCC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	endif()
else()
	message(FATAL_ERROR "Unsupported CPU: ${TARGET_CPU}")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	set(GCC_MIN_VER "9.1")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL ${GCC_MIN_VER})
		message(STATUS "GNUCXX Found, version: ${CMAKE_CXX_COMPILER_VERSION}")
	else()
		message(FATAL_ERROR "GCC version ${GCC_MIN_VER} required. System version is ${CMAKE_CXX_COMPILER_VERSION}")
	endif()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(BUILD_TESTS OFF CACHE BOOL "Build unit tests")
set(BUILD_BENCHMARKS OFF CACHE BOOL "Build benchmarks")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist/bin)

set(FSDB_SOURCES
		src/fsdb.cpp
		)

set(FSDB_HEADERS
		include/fsdb/fsdb.hpp
		)


add_library(fsdb STATIC ${FSDB_SOURCES} ${FSDB_HEADERS})
target_compile_features(fsdb PUBLIC cxx_std_17)
set_target_properties(fsdb PROPERTIES PUBLIC_HEADER "${FSDB_HEADERS}")

set(FSDB_INCLUDE_DIRS include)
set(FSDB_LIBS fsdb)

target_include_directories(fsdb PUBLIC ${FSDB_INCLUDE_DIRS})

if(BUILD_TESTS)
	configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
	if(result)
		message(FATAL_ERROR "CMake step for googletest failed: ${result}")
	endif()
	execute_process(COMMAND ${CMAKE_COMMAND} --build .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
	if(result)
		message(FATAL_ERROR "Build step for googletest failed: ${result}")
	endif()

	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

	add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
			${CMAKE_CURRENT_BINARY_DIR}/googletest-build
			EXCLUDE_FROM_ALL)

    include_directories("${gtest_SOURCE_DIR}/include")
    include_directories("${gmock_SOURCE_DIR}/include")

	add_executable(run_all_tests
			#tests/run_all_tests.cpp
			tests/ut_basic.cpp
			)
	target_include_directories(run_all_tests PRIVATE ${FSDB_INCLUDE_DIRS})
	target_link_libraries(run_all_tests fsdb gtest_main gmock_main)
	enable_testing()
	add_test(NAME unit_tests COMMAND run_all_tests)
endif()

if(BUILD_BENCHMARKS)
	configure_file(CMakeLists.txt.benchmark.in googlebenchmark-download/CMakeLists.txt)
	execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release -G "${CMAKE_GENERATOR}" .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googlebenchmark-download)
	if(result)
		message(FATAL_ERROR "CMake step for google benchmark failed: ${result}")
	endif()
	execute_process(COMMAND ${CMAKE_COMMAND} --build .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googlebenchmark-download)
	if(result)
		message(FATAL_ERROR "Build step for google benchmark failed: ${result}")
	endif()

	add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googlebenchmark-src
			${CMAKE_CURRENT_BINARY_DIR}/googlebenchmark-build
			EXCLUDE_FROM_ALL)

	include_directories("${benchmark_SOURCE_DIR}/include")
	add_executable(run_benchmarks
			benchmarks/main.cpp
			)
	set_target_properties(run_benchmarks PROPERTIES CXX_STANDARD 17)
	target_link_libraries(run_benchmarks fsdb benchmark)
endif()

install(TARGETS fsdb
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		PUBLIC_HEADER DESTINATION include/fsdb)

add_executable(main main.cpp)
set_target_properties(main PROPERTIES OUTPUT_NAME fsdb)
target_link_libraries(main fsdb)
