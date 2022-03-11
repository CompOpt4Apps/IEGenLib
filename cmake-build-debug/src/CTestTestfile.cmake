# CMake generated Testfile for 
# Source directory: /tmp/tmp.8uD4RXirDP/src
# Build directory: /tmp/tmp.8uD4RXirDP/cmake-build-debug/src
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(iegenlib_unit_test "iegenlib_t")
set_tests_properties(iegenlib_unit_test PROPERTIES  ENVIRONMENT "IEGEN_HOME=/tmp/tmp.8uD4RXirDP/cmake-build-debug" _BACKTRACE_TRIPLES "/tmp/tmp.8uD4RXirDP/src/CMakeLists.txt;198;add_test;/tmp/tmp.8uD4RXirDP/src/CMakeLists.txt;0;")
subdirs("bindings")
