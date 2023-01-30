#!/usr/bin/env cmake

#
# This script is used on Linux to clear (if needed) and
# mark a new tag release based on
#    cmake/version.cmake
#
# and then run a docker build.
#
# To run it, from the main directory run:
#
#    cmake -P cmake/github_release.cmake
#

include( cmake/version.cmake )

find_package( Git REQUIRED )


#
# Delete local tag if available
#
message( STATUS "Remove local tag v${mrv2_VERSION}" )
execute_process( COMMAND ${GIT_EXECUTABLE} tag -d v${mrv2_VERSION}
		 OUTPUT_VARIABLE _output  )
message( STATUS "${_output}" )

#
# Delete remote tag if available
#
message( STATUS "Remove remote tag v${mrv2_VERSION}" )
execute_process( COMMAND ${GIT_EXECUTABLE} push --delete origin v${mrv2_VERSION}                 OUTPUT_VARIABLE _output )
message( STATUS "${_output}" )


#
# Mark current repository with a new tag
#
message( STATUS "Create local tag v${mrv2_VERSION}" )
execute_process( COMMAND ${GIT_EXECUTABLE} tag v${mrv2_VERSION}
		 OUTPUT_VARIABLE _output )
message( STATUS "${_output}" )

#
# Send new tag to repository
#
message( STATUS "Create remote tag v${mrv2_VERSION}" )
execute_process( COMMAND ${GIT_EXECUTABLE} push origin v${mrv2_VERSION}
		 OUTPUT_VARIABLE _output )
message( STATUS "${_output}" )

#
# Remove the main images if present
#
message( STATUS "Remove rockylinux image..." )
execute_process( COMMAND docker rmi rockylinux:8 --force )

message( STATUS "Remove docker image..." )
execute_process( COMMAND docker rmi mrv2_builder:latest --force )

#
# Remove dangling images
#
message( STATUS "Remove dangling image..." )
execute_process( COMMAND "docker rmi $(docker images -a --filter=dangling=true -q)" )


message( STATUS "Run a docker build..." )
execute_process( COMMAND runme_docker.sh )
