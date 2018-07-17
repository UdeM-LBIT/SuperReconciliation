cmake_minimum_required(VERSION 3.1)
project(super_reconciliation)

# Compiler settings
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Dependencies
find_package(Boost 1.60 COMPONENTS program_options REQUIRED)
find_package(OpenMP REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

# Common library
add_library(common
    src/algo/erase.cpp
    src/algo/super_reconciliation.cpp
    src/algo/unordered_super_reconciliation.cpp
    src/io/nhx_parser.cpp
    src/io/util.cpp
    src/model/Event.cpp
    src/model/Synteny.cpp
)

target_link_libraries(common PUBLIC ${Boost_LIBRARIES})
target_include_directories(common PUBLIC lib)
target_include_directories(common PUBLIC ${Boost_INCLUDE_DIR})

# `reconcile` executable
add_executable(reconcile src/reconcile.cpp)
target_link_libraries(reconcile common)
target_include_directories(reconcile PUBLIC lib)
target_include_directories(reconcile PUBLIC ${Boost_INCLUDE_DIR})

# `simulate` executable
add_executable(simulate src/simulate.cpp)
target_link_libraries(simulate common)
target_include_directories(simulate PUBLIC lib)
target_include_directories(simulate PUBLIC ${Boost_INCLUDE_DIR})

# `evaluate` executable
add_executable(evaluate src/evaluate.cpp)
target_link_libraries(evaluate common)
target_include_directories(evaluate PUBLIC lib)
target_include_directories(evaluate PUBLIC ${Boost_INCLUDE_DIR})

# `erase` executable
add_executable(erase src/erase.cpp)
target_link_libraries(erase common)
target_include_directories(erase PUBLIC lib)
target_include_directories(erase PUBLIC ${Boost_INCLUDE_DIR})

# `viz` executable
add_executable(viz src/viz.cpp)
target_link_libraries(viz common)
target_include_directories(viz PUBLIC lib)
target_include_directories(viz PUBLIC ${Boost_INCLUDE_DIR})

# `tests` executable
add_executable(tests
    src/tests.cpp
    src/io/nhx_parser.test.cpp
    src/model/Synteny.test.cpp
    src/util/ExtendedNumber.test.cpp
    src/util/MultivaluedNumber.test.cpp
)

target_link_libraries(tests common)
target_include_directories(tests PUBLIC lib)
target_include_directories(tests PUBLIC ${Boost_INCLUDE_DIR})
