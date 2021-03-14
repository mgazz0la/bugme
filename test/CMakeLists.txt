include(FetchContent)
FetchContent_Declare(gtest
  QUIET
  URL https://github.com/google/googletest/archive/release-1.10.0.tar.gz
)
# configure build of googletest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gtest)
include(GoogleTest)

macro(package_add_test_with_libraries TESTNAME FILES LIBRARIES)
    add_executable(${TESTNAME} ${FILES})
    target_link_libraries(${TESTNAME} PRIVATE gtest_main ${LIBRARIES})
    gtest_discover_tests(${TESTNAME}
      PROPERTIES
        LABELS "unit"
      DISCOVERY_TIMEOUT  # how long to wait (in seconds) before crashing
        240
      )
endmacro()

#package_add_test_with_libraries(register_test register_test.cc register)

add_executable(register_test register_test.cc)
target_link_libraries(register_test PRIVATE register gtest_main)
gtest_discover_tests(register_test
  PROPERTIES
    LABELS "unit"
  DISCOVERY_TIMEOUT  # how long to wait (in seconds) before crashing
    240
  )

add_executable(cpu_test cpu_test.cc)
target_link_libraries(cpu_test PRIVATE cpu gtest_main)
gtest_discover_tests(cpu_test
  PROPERTIES
    LABELS "unit"
  DISCOVERY_TIMEOUT  # how long to wait (in seconds) before crashing
    240
  )

