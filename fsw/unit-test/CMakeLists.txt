##################################################################
#
# Coverage Unit Test build recipe
#
# This CMake file contains the recipe for building the server unit tests.
# It is invoked from the parent directory when unit tests are enabled.
#
##################################################################

#
#
# NOTE on the subdirectory structures here:
#
# - "inc" provides local header files shared between the coveragetest,
#    wrappers, and overrides source code units
# - "coveragetest" contains source code for the actual unit test cases
#    The primary objective is to get line/path coverage on the FSW 
#    code units.
#
 
# Use the UT assert public API, and allow direct
# inclusion of source files that are normally private
include_directories(${PROJECT_SOURCE_DIR}/fsw/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${hwlib_MISSION_DIR}/fsw/public_inc)


# Add a coverage test executable called "server-ALL" that 
# covers all of the functions in server_app.  
#
# Also note in a more complex app/lib the coverage test can also
# be broken down into smaller units (in which case one should use
# a unique suffix other than "ALL" for each unit).  For example,
# OSAL implements a separate coverage test per source unit.
add_cfe_coverage_test(server ALL 
    "coveragetest/coveragetest_server_app.c"
    "../src/server_app.c"
    "../src/server_device.c"
    "../../../../fsw/apps/hwlib/fsw/stubs/libuart.c"
)

# The server uses library functions provided by server_lib so must be linked
# with the server_lib stub library (this is mainly just an example of how this 
# can be done).
#add_cfe_coverage_dependency(server ALL server_lib)

