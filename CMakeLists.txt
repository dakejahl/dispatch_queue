# Define library. Only source files here!
cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(dispatch_queue)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# If no install directory is supplied by user of library, use GNUInstallDirs
if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
    include(GNUInstallDirs)
	message(STATUS "No install DESTINATION, using GNUInstallDirs (${CMAKE_INSTALL_LIBDIR})")
else()
	message(STATUS "Using install DESTINATION ${CMAKE_INSTALL_LIBDIR}")	
endif()

# Add library source
add_library(dispatch_queue
    src/DispatchQueue.cpp)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(dispatch_queue PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

# If we have compiler requirements for this library, list them
# here
# target_compile_features(lib
#     PUBLIC cxx_auto_type
#     PRIVATE cxx_variadic_templates)

# Depend on a library that we defined in the top-level file
# target_link_libraries(mylib1
#     boost
#     MyOtherLibrary)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS dispatch_queue EXPORT dispatch_queue_config
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  
# This is for Windows
# install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT dispatch_queue_config DESTINATION share/dispatch_queue/cmake)

# This makes the project importable from the build directory
export(TARGETS dispatch_queue FILE dispatch_queue_config.cmake)

# Every library has unit tests, of course
enable_testing()

add_executable(test_main
    test/test_main.cpp)

add_executable(test_shared_resource
    test/test_shared_resource.cpp)

target_link_libraries(test_main
    dispatch_queue)

target_link_libraries(test_shared_resource
    dispatch_queue)

add_test(test_main test_main)
add_test(test_shared_resource test_shared_resource)
