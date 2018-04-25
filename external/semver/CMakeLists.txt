cmake_minimum_required (VERSION 2.8)
project (semver)

# Includes for this project
include_directories ("${PROJECT_SOURCE_DIR}/src/include")

# Include testinator and the GSL
include_directories ("${PROJECT_SOURCE_DIR}/contrib/testinator/src/include")
include_directories ("${PROJECT_SOURCE_DIR}/contrib/gsl/include")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default C++ standard: C++14
if(CXX_STD)
else()
  set(CXX_STD 14)
endif()

# Set up tests
enable_testing()
include(CTest)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(MY_CXX_FLAGS_LIST
    )
  string(REPLACE ";" " " MY_CXX_FLAGS "${MY_CXX_FLAGS_LIST}")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MY_CXX_FLAGS}")
else()
  set(MY_CXX_FLAGS_LIST
    -ftemplate-backtrace-limit=0
    -ffunction-sections
    -Wall -Wextra -Werror -pedantic-errors
    -Wcast-align
    -Wcast-qual
    -Wctor-dtor-privacy
    -Wdisabled-optimization
    -Wformat=2
    -Winit-self
    -Wmissing-include-dirs
    # -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    # -Wshadow
    # -Wsign-conversion
    -Wsign-promo
    -Wstrict-overflow=2
    -Wswitch-default
    -Wundef
    )
  string(REPLACE ";" " " MY_CXX_FLAGS "${MY_CXX_FLAGS_LIST}")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++${CXX_STD} ${MY_CXX_FLAGS}")

  # Debug/Release
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-inline -g3 -fstack-protector-all")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -g0 -march=native -mtune=native -DNDEBUG")
  set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
endif()

# Clang/GCC specifics
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  if(SAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined,integer -fno-omit-frame-pointer -fno-sanitize=unsigned-integer-overflow")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCXX)
endif()

# Pipe separate tests into ctest
# Adapted from https://github.com/ChaiScript/ChaiScript/blob/develop/CMakeLists.txt
macro(ADD_INDIVIDUAL_TESTS executable type suffix)
  set(test_path $ENV{PATH})
  get_target_property(target_files ${executable} SOURCES)
  foreach(source ${target_files})
    string(REGEX MATCH .*cpp source "${source}")
    if(source)
      file(READ "${source}" contents)
      string(REGEX MATCHALL "DEF_${type}[ ]*[(][ ]*[^, ]+[ ]*,[ ]*[^,) ]+[ ]*[),]" found_tests ${contents})
      foreach(hit ${found_tests})
        string(REGEX REPLACE "DEF_${type}[ ]*[(][ ]*([^, ]+)[ ]*,[ ]*[^,) ]+[ ]*[),]" "\\1" tname ${hit})
        string(REGEX REPLACE "DEF_${type}[ ]*[(][ ]*[^, ]+[ ]*,[ ]*([^,) ]+)[ ]*[),]" "\\1" sname ${hit})
        set(test_name ${executable}.${sname}.${tname}${suffix})
        add_test(NAME ${test_name}
          COMMAND "${executable}" --testName=${tname}${suffix} --suiteName=${sname})
        set_tests_properties(${test_name} PROPERTIES TIMEOUT 30 ENVIRONMENT "PATH=${test_path}")
      endforeach()
    endif()
  endforeach()
endmacro()

macro(ADD_TESTINATOR_TESTS executable)
  ADD_INDIVIDUAL_TESTS(${executable} "TEST" "")
  ADD_INDIVIDUAL_TESTS(${executable} "TIMED_TEST" "")
  ADD_INDIVIDUAL_TESTS(${executable} "PROPERTY" "Property")
  ADD_INDIVIDUAL_TESTS(${executable} "COMPLEXITY_PROPERTY" "ComplexityProperty")
endmacro()

add_subdirectory (src/lib)
add_subdirectory (src/test)
add_subdirectory (src/quickcheck)
