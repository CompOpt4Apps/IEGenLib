# CMake generated Testfile for 
# Source directory: /Users/ant/Documents/AdaptLab/IEGenLib/src
# Build directory: /Users/ant/Documents/AdaptLab/IEGenLib/cmake-build-debug/src
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(iegenlib_unit_test "iegenlib_t")
set_tests_properties(iegenlib_unit_test PROPERTIES  ENVIRONMENT "IEGEN_HOME=/Users/ant/Documents/AdaptLab/IEGenLib/cmake-build-debug" _BACKTRACE_TRIPLES "/Users/ant/Documents/AdaptLab/IEGenLib/src/CMakeLists.txt;198;add_test;/Users/ant/Documents/AdaptLab/IEGenLib/src/CMakeLists.txt;0;")
subdirs("bindings")
