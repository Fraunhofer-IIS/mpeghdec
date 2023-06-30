# search for doxygen
find_package(Doxygen)

if(DOXYGEN_FOUND)
  set (DOC_SRC_FILES 
    ${CMAKE_SOURCE_DIR}/doc/mainpage.md
    ${CMAKE_SOURCE_DIR}/include/mpeghdecoder.h
    ${CMAKE_SOURCE_DIR}/include/mpeghUIManager.h
  )
  
  # Set the doxygen main page (also used in gitlab)
  set (MAINPAGE_MD ${CMAKE_SOURCE_DIR}/doc/mainpage.md)

  # Set the oxygen project brief
  set (DOC_PROJECT_BRIEF "Fraunhofer IIS MPEG-H Decoder Library")

  # convert semicolon separated list into list with whitespaces
  string(REGEX REPLACE ";" " " DOC_SRC_FILES "${DOC_SRC_FILES}")
  
  # configure input and output for doxy file
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  
  # write doxy file and replace placeholders
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
   
  # add internal doxygen documentation
  add_custom_target(
    ${PROJECT_NAME}_doc ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Create doxygen documentation"
    VERBATIM
  )

  add_custom_command(
    TARGET ${PROJECT_NAME}_doc
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/latex
    COMMENT "Create doxygen PDF documentation"
    VERBATIM
    DEPENDS ${PROJECT_NAME}_doc
  )
else()
  message("Doxygen need to be installed to generate the doxygen documentation")
endif()
