add_library(embed_python STATIC src/Module.cxx)

target_include_directories(
  embed_python PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:>
)

target_link_libraries(embed_python PUBLIC Python3::Python)

add_executable(test_embed_python src/test/runPy.cxx)
target_link_libraries(test_embed_python PRIVATE embed_python)
