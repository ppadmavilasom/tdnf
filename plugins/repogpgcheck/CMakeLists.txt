#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

project(tdnfrepogpgcheck)

add_library(${PROJECT_NAME} SHARED
    api.c
    repogpgcheck.c
)

include_directories(${CMAKE_SOURCE_DIR}/include)

#target_link_libraries(
#    ${CMAKE_BINARY_DIR}/lib/libtdnf.so
#)

set_target_properties(${PROJECT_NAME} PROPERTIES
   LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
