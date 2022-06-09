add_library(embed_python src/Module.cxx)

target_include_directories(
  embed_python PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:>
)

target_link_libraries(embed_python PUBLIC Python3::Python)

add_executable(test_embed_python src/test/runPy.cxx)
target_link_libraries(test_embed_python PRIVATE embed_python)

###############################################################
# Installation
###############################################################
install(DIRECTORY embed_python DESTINATION ${FERMI_INSTALL_INCLUDEDIR})
install(DIRECTORY python/ DESTINATION ${FERMI_INSTALL_PYTHON})

install(
  TARGETS embed_python test_embed_python
  # EXPORT fermiTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
