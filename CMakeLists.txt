#----------------------------------------------------------------------#
# SEAR API build script                                                #
#                                                                      #
# This script can build:                                               #
# - SEAR                                                               #
#   - as an archive library (libsear.a)                                #
#     cmake --preset <your preset>                                     #
#     cmake --build --preset <your preset> --target sear               #
#                                                                      #
#   - as a shared library (libsear.so + libsear.x)                     #
#     cmake --preset <your preset> -DBUILD_SHARED_LIBS=on              #
#     cmake --build --preset <your preset> --target sear               #
#                                                                      #
# - pysear                                                             #
#   - as a python wheel with sear incorporated                         #
#     python -m build .                                                #
#                                                                      #
# - test_runner                                                        #
#   - as a stand alone executable (that requires libsear.so)           #
#     cmake --preset linux-test -DBUILD_SHARED_LIBS=on                 #
#     cmake --build --preset linux-test --target sear                  #
#     cmake --build --preset linux-test --target test_runner           #
#                                                                      #   
# - bench_runner                                                       #
#   - as a stand alone executable (that requires libsear.so)           #
#     cmake --preset linux-bench -DBUILD_SHARED_LIBS=on                #
#     cmake --build --preset linux-bench --target sear                 #
#     cmake --build --preset linux-bench --target bench_runner         #
#                                                                      #
# - debug                                                              #
#   - as a stand alone executable                                      #
#     cmake --preset zos -DBUILD_SHARED_LIBS=on                        #
#     cmake --build --preset zos --target sear                         #
#     cmake --build --preset zos --target debug                        #
#                                                                      #
# - lint                                                               #
#   - executes clang-format for most .cpp and .hpp files,              #
#     this is executed in a Github workflow                            #
#                                                                      #
# - check                                                              #
#   - execute cppcheck on the sear code,                               #
#     this is executed in a Github workflow                            #
#                                                                      #
# - fuzz                                                               #
#   - compile sear code with fuzz and run fuzz security checks         #
#     this is exectuted in a Github workflow                           #
#----------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.24)

project(sear C CXX ASM)
set(CXX_STANDARD 17)

# This option is used when building the Python wheel
option(SEAR_ENABLE_PYTHON "Enables building the Python language module." OFF)

# This option is used on Linux to build SEAR with mocked unit tests
option(SEAR_UNIT_TEST "Builds tests" OFF)

option(SEAR_BUILD_BENCHMARKS "Build performance benchmarks" OFF)

find_package(OpenSSL REQUIRED)

set(SEAR_SOURCES
    sear/irrsdl00/irrsdl00.cpp
    sear/irrsdl00/keyring_extractor.cpp
    sear/irrsdl00/keyring_modifier.cpp
    sear/irrsdl00/keyring_post_processor.cpp
    sear/irrseq00/profile_extractor.cpp
    sear/irrseq00/profile_post_processor.cpp
    sear/irrsmo00/irrsmo00_error.cpp
    sear/irrsmo00/irrsmo00.cpp
    sear/irrsmo00/xml_generator.cpp
    sear/irrsmo00/xml_parser.cpp
    sear/key_map/key_map.cpp
    sear/validation/trait_validation.cpp
    sear/logger.cpp
    sear/sear_error.cpp
    sear/sear.cpp
    sear/security_admin.cpp
    sear/security_request.cpp
    sear/conversion.cpp
    externals/json-schema-validator/json-patch.cpp
    externals/json-schema-validator/json-schema-draft7.json.cpp
    externals/json-schema-validator/json-uri.cpp
    externals/json-schema-validator/json-validator.cpp
    externals/json-schema-validator/smtp-address-validator.cpp
    externals/json-schema-validator/string-format-check.cpp
    externals/pugixml/pugixml.cpp
)

set(SEAR_INCLUDE_DIRS
    sear
    sear/irrsdl00
    sear/irrseq00
    sear/irrsmo00
    sear/key_map
    sear/validation
    externals/json
    externals/json-schema-validator
    externals/iconv
    externals/pugixml
)

set(SEAR_MOCK_SOURCES
    tests/mock/irrsdl64.cpp
    tests/mock/irrseq00.cpp
    tests/mock/irrsmo64.cpp
)

set(TEST_SUPPORT_SOURCES
    tests/unit_test_utilities.cpp
    tests/unity/unity.c
    tests/zoslib/zoslib.cpp
)

if (SEAR_UNIT_TEST)
    set(SEAR_PLATFORM_INCLUDE_DIRS tests/zoslib)
    set(SEAR_DEBUG_OPTIONS -g)
else()
    find_library(SEAR_ZOSLIB libzoslib.a REQUIRED)
    set(SEAR_PLATFORM_SOURCES sear/irrseq00/irrseq00.s)
endif()

file(READ schema.json SEAR_SCHEMA)
configure_file(sear/sear_schema.hpp.in sear/sear_schema.hpp)

add_library(sear ${SEAR_SOURCES} ${SEAR_PLATFORM_SOURCES})
target_include_directories(
    sear PUBLIC
    ${SEAR_INCLUDE_DIRS}
    ${SEAR_PLATFORM_INCLUDE_DIRS}
    "${CMAKE_CURRENT_BINARY_DIR}/sear"
)
target_compile_definitions(sear PUBLIC _POSIX_C_SOURCE=200112L)
target_compile_options(sear PUBLIC ${SEAR_DEBUG_OPTIONS})
target_link_libraries(sear PRIVATE OpenSSL::SSL ${SEAR_ZOSLIB})

if (BUILD_SHARED_LIBS AND NOT SEAR_UNIT_TEST)
    target_link_options(sear PRIVATE -Wl,-e,CELQSTRT)
endif()

# If we're building the Python wheel
if (SEAR_ENABLE_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Development Development.SABIModule)

    Python3_add_library(pysear MODULE USE_SABI 3.12 sear/python/_sear.c)
    target_link_libraries(pysear PRIVATE sear Python3::Python)

    if (DEFINED SEAR_PYTHON_EXTENSION_PATH)
        message(STATUS "Installing Python module to: ${SEAR_PYTHON_EXTENSION_PATH}")
        cmake_path(GET SEAR_PYTHON_EXTENSION_PATH FILENAME MODULE_FILENAME)
        cmake_path(GET SEAR_PYTHON_EXTENSION_PATH PARENT_PATH MODULE_DIR)
        install(PROGRAMS "$<TARGET_FILE:pysear>" DESTINATION "${MODULE_DIR}" RENAME "${MODULE_FILENAME}")
    endif()
endif()

add_executable(
    test_runner
    tests/test_runner.cpp
    tests/irrsdl00/test_irrsdl00.cpp
    tests/irrseq00/test_irrseq00.cpp
    tests/irrsmo00/test_irrsmo00.cpp
    tests/validation/test_parameter_validation.cpp
    ${SEAR_MOCK_SOURCES}
    ${TEST_SUPPORT_SOURCES}
)
target_include_directories(
    test_runner PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SEAR_INCLUDE_DIRS}
    tests/irrsdl00
    tests/zoslib
)
target_compile_definitions(test_runner PUBLIC UNIT_TEST UNITY_OUTPUT_COLOR)
target_compile_options(test_runner PUBLIC ${SEAR_DEBUG_OPTIONS})
target_link_libraries(test_runner PRIVATE sear)

if (SEAR_BUILD_BENCHMARKS)
    add_executable(
        bench_runner
        tests/bench_runner.cpp
        ${SEAR_MOCK_SOURCES}
        ${TEST_SUPPORT_SOURCES}
    )
    target_include_directories(
        bench_runner PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SEAR_INCLUDE_DIRS}
        tests/zoslib
    )

    target_compile_options(bench_runner PUBLIC -O3)
    target_compile_definitions(bench_runner PUBLIC BENCHMARK_MODE)
    target_link_libraries(bench_runner PRIVATE sear OpenSSL::SSL)
endif()

# The debug executable
add_executable(
    debug
    debug/debug.c
)

# Custom target 'link', executes clang-format for most .cpp and .hpp files
add_custom_target(lint ALL
    COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/sear/ -type f -name "*.cpp" -exec clang-format-19 --verbose --Werror --dry-run -i {} "\;"
    COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/sear/ -type f -name "*.hpp" -exec clang-format-19 --verbose --Werror --dry-run -i {} "\;"
    COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/tests/ -type f -name "*.cpp" -exec clang-format-19 --verbose --Werror --dry-run -i {} "\;"
    COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/tests/ -type f -name "*.hpp" -exec clang-format-19 --verbose --Werror --dry-run -i {} "\;"
)

# Custom target 'check', executes 'cppcheck' on the sear code
add_custom_target(check ALL
    COMMAND "cppcheck"
                "--suppress='missingIncludeSystem'"
                "--suppress='useStlAlgorithm'"
                "--inline-suppr"
                "--language=c++"
                "--std=c++${CXX_STANDARD}"
                "--enable=all"
                "--force"
                "--check-level=exhaustive"
                "--inconclusive"
                "--error-exitcode=1"
                "-D __ptr32="
                "-D _POSIX_C_SOURCE=200112L"
                "-I${CMAKE_CURRENT_SOURCE_DIR}/sear"
                "-I${CMAKE_CURRENT_SOURCE_DIR}/sear/irrsdl00"
                "-I${CMAKE_CURRENT_SOURCE_DIR}/sear/irrseq00"
                "-I${CMAKE_CURRENT_SOURCE_DIR}/sear/irrsmo00"
                "-I${CMAKE_CURRENT_SOURCE_DIR}/sear/key_map"
                "-I${CMAKE_CURRENT_SOURCE_DIR}/sear/validation"
                "-I${CMAKE_CURRENT_SOURCE_DIR}/tests/zoslib"
                "-I${CMAKE_CURRENT_BINARY_DIR}/sear"
                "${CMAKE_CURRENT_SOURCE_DIR}/sear"
)

add_executable(
    fuzz
    tests/fuzz.cpp
    ${SEAR_MOCK_SOURCES}
    tests/zoslib/zoslib.cpp
    ${SEAR_SOURCES}
)
target_include_directories(
    fuzz PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SEAR_INCLUDE_DIRS}
    tests/irrsdl00
    tests/zoslib
    "${CMAKE_CURRENT_BINARY_DIR}/sear"
)
target_compile_options(fuzz PUBLIC -fsanitize=fuzzer -fsanitize=undefined -fsanitize=address)
target_link_options(fuzz PUBLIC -fsanitize=fuzzer -fsanitize=undefined -fsanitize=address)
target_link_libraries(fuzz PRIVATE OpenSSL::SSL ${SEAR_ZOSLIB})
