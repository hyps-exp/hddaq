set(MY_TARGET HulCore)
set(MY_TARGET_EXPORT HulCore-Export)
set(MY_PUB_HEADER
  "rbcp.hh"
  "UDPRBCP.hh"
  "FPGAModule.hh"
  "Utility.hh"
  "BitDump.hh"
  "DaqFuncs.hh"  
  )

file(GLOB_RECURSE SRCS "*.cc")

add_library(${MY_TARGET} SHARED
  ${SRCS}
  )

target_compile_options(${MY_TARGET} PRIVATE
  ${my_cxx_options}
  )
target_compile_features(${MY_TARGET} PRIVATE
  ${my_cxx_features}
  )
target_include_directories(${MY_TARGET} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR};
  )

set_target_properties(${MY_TARGET} PROPERTIES
  PUBLIC_HEADER "${MY_PUB_HEADER}"
  )

install(TARGETS ${MY_TARGET}
  EXPORT        ${MY_TARGET_EXPORT}
  LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

install(EXPORT ${MY_TARGET_EXPORT}
  FILE ${MY_TARGET}Config.cmake
  DESTINATION share/cmake/${MY_TARGET}
  EXPORT_LINK_INTERFACE_LIBRARIES
  )
