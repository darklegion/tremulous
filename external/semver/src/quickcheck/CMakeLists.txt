find_program(HSC2HS hsc2hs)
find_program(GHC ghc)

if (HSC2HS AND GHC)

  # the shared lib
  add_library(${PROJECT_NAME}_ffi SHARED
    ../lib/semantic_version_v1.cpp ../lib/semantic_version_v2.cpp semantic_version_ffi.cpp)
  # the ffi bindings
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Semver.hs
    MAIN_DEPENDENCY Semver.hsc
    DEPENDS semantic_version_ffi.h
    COMMAND ${HSC2HS} ${CMAKE_CURRENT_SOURCE_DIR}/Semver.hsc -I ${CMAKE_CURRENT_SOURCE_DIR} -o ${CMAKE_CURRENT_BINARY_DIR}/Semver.hs)
  # the quickcheck executable
  add_custom_command(
    OUTPUT quickcheck_${PROJECT_NAME}
    MAIN_DEPENDENCY Main.hs
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Semver.hs
    DEPENDS ${PROJECT_NAME}_ffi
    COMMAND ${GHC} --make ${CMAKE_CURRENT_SOURCE_DIR}/Main -L. -lsemver_ffi -o quickcheck_${PROJECT_NAME})
  add_custom_target(quickcheck ALL
    DEPENDS quickcheck_${PROJECT_NAME})
  # pipe to ctest
  add_test(NAME quickcheck_${PROJECT_NAME}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/quickcheck_${PROJECT_NAME})
  set_tests_properties(quickcheck_${PROJECT_NAME} PROPERTIES TIMEOUT 30 ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}")

endif()
