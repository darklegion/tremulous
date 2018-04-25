add_executable (test_${PROJECT_NAME} main.cpp)
target_link_libraries(test_${PROJECT_NAME} ${PROJECT_NAME})
ADD_TESTINATOR_TESTS (test_${PROJECT_NAME})
