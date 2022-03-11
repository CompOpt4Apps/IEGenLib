
if(NOT "/tmp/tmp.8uD4RXirDP/cmake-build-debug-remote/lib-omega-prefix/src/lib-omega-stamp/lib-omega-gitinfo.txt" IS_NEWER_THAN "/tmp/tmp.8uD4RXirDP/cmake-build-debug-remote/lib-omega-prefix/src/lib-omega-stamp/lib-omega-gitclone-lastrun.txt")
  message(STATUS "Avoiding repeated git clone, stamp file is up to date: '/tmp/tmp.8uD4RXirDP/cmake-build-debug-remote/lib-omega-prefix/src/lib-omega-stamp/lib-omega-gitclone-lastrun.txt'")
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E remove_directory "/tmp/tmp.8uD4RXirDP/lib/omega"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/tmp/tmp.8uD4RXirDP/lib/omega'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/local/apps/git/bin/git"  clone --progress "https://github.com/BoiseState-AdaptLab/omega" "omega"
    WORKING_DIRECTORY "/tmp/tmp.8uD4RXirDP/lib"
    RESULT_VARIABLE error_code
    )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once:
          ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/BoiseState-AdaptLab/omega'")
endif()

execute_process(
  COMMAND "/usr/local/apps/git/bin/git"  checkout e0e5d046dac0b99cbf17711f168728081b23dea9 --
  WORKING_DIRECTORY "/tmp/tmp.8uD4RXirDP/lib/omega"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'e0e5d046dac0b99cbf17711f168728081b23dea9'")
endif()

execute_process(
  COMMAND "/usr/local/apps/git/bin/git"  submodule update --recursive --init 
  WORKING_DIRECTORY "/tmp/tmp.8uD4RXirDP/lib/omega"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/tmp/tmp.8uD4RXirDP/lib/omega'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy
    "/tmp/tmp.8uD4RXirDP/cmake-build-debug-remote/lib-omega-prefix/src/lib-omega-stamp/lib-omega-gitinfo.txt"
    "/tmp/tmp.8uD4RXirDP/cmake-build-debug-remote/lib-omega-prefix/src/lib-omega-stamp/lib-omega-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/tmp/tmp.8uD4RXirDP/cmake-build-debug-remote/lib-omega-prefix/src/lib-omega-stamp/lib-omega-gitclone-lastrun.txt'")
endif()

