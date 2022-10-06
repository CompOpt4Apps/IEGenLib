# CMake generated Testfile for 
# Source directory: /home/kbwal/IEGenLib/src
# Build directory: /home/kbwal/IEGenLib/cmake-build-debug/src
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(iegenlib_unit_test "iegenlib_t")
set_tests_properties(iegenlib_unit_test PROPERTIES  ENVIRONMENT "IEGEN_HOME=/home/kbwal/IEGenLib/cmake-build-debug" _BACKTRACE_TRIPLES "/home/kbwal/IEGenLib/src/CMakeLists.txt;198;add_test;/home/kbwal/IEGenLib/src/CMakeLists.txt;0;")
subdirs("bindings")
