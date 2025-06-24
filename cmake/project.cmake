function(project_add_documentation_target)
  if (TARGET ${PROJECT_NAME}_doc)
    return()
  endif ()
  cmake_parse_arguments(PARAM "" "MARKDOWN;TITLE" "" ${ARGN})

  find_program(PLANTUML NAMES plantuml)
  find_program(PANDOC NAMES pandoc)
  add_custom_target(
    ${PROJECT_NAME}_doc
    mkdir -p ${CMAKE_BINARY_DIR}/doc/images &&
    ${PLANTUML} ${CMAKE_SOURCE_DIR}/documentation/images/*.plantuml &&
    ${PANDOC} -s --from gfm --to html --filter pandoc-plantuml -o ${CMAKE_BINARY_DIR}/doc/${PROJECT_NAME}_doc.html --css style.css ${PARAM_MARKDOWN} --metadata title='${PARAM_TITLE}' &&
    cmake -E copy_if_different ${CMAKE_SOURCE_DIR}/documentation/style.css doc/ &&
    cmake -E copy_if_different ${CMAKE_SOURCE_DIR}/documentation/images/*.png doc/images/
  )
endfunction()
