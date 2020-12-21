# CMake generated Testfile for 
# Source directory: /mnt/c/Users/shivi/Desktop/IEGenLib/src
# Build directory: /mnt/c/Users/shivi/Desktop/IEGenLib/build/src
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(iegenlib_unit_test "iegenlib_t")
set_tests_properties(iegenlib_unit_test PROPERTIES  ENVIRONMENT "IEGEN_HOME=/mnt/c/Users/shivi/Desktop/IEGenLib/build")
subdirs("bindings")
