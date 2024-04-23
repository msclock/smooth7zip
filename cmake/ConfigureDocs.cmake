#[[
This file creates targets to generate documentation:

   - Doxygen: ${CMAKE_PROJECT_NAME}-doxygen: generates doxygen output.
   - Sphinx:
       - ${CMAKE_PROJECT_NAME}-docs: generates HTML documentation.
       - ${CMAKE_PROJECT_NAME}-docs-check: generates HTML documentation with warnings as errors.
       - ${CMAKE_PROJECT_NAME}-docs-linkcheck: checks links in HTML documentation
]]

include(cmake-modules/configure/ConfigDoxygen)
configdoxygen()

if(DOXYGEN_FOUND)
  set(DOXYGEN_GENERATE_HTML
      NO
      CACHE STRING "Generate HTML output")
  set(DOXYGEN_GENERATE_XML
      YES
      CACHE STRING "Generate XML output for breathe")
  set(DOXYGEN_QUIET
      NO
      CACHE BOOL "Quiet mode")
  set(DOXYGEN_REFERENCED_BY_RELATION
      YES
      CACHE BOOL "Show referenced by relations")
  set(DOXYGEN_REFERENCES_LINK_SOURCE
      YES
      CACHE BOOL "Link to source code from references")
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/docs/_build/doxygen)

  # Add XML generation doxygen target
  set(doxygen_docs ${CMAKE_PROJECT_NAME}-doxygen)
  # cmake-format: off
  doxygen_add_docs(
    ${doxygen_docs}
    "${CMAKE_BINARY_DIR}/git/include"
    "${PROJECT_SOURCE_DIR}/src/smooth7zip/include")
  # cmake-format: on

  if(TARGET ${doxygen_docs})
    set(Sphinx_FIND_COMPONENTS breathe)
    include(cmake-modules/configure/FindSphinx)
    unset(Sphinx_FIND_COMPONENTS)

    set(DOCS_CHECK_NITPICKY_MODE
        OFF
        CACHE BOOL "Nitpicky mode for Sphinx documentation")

    set(_common_sphinx_args
        CONF_FILE
        ${PROJECT_SOURCE_DIR}/docs/conf.py
        BREATHE_PROJECTS
        ${doxygen_docs}
        BREATH_DEBUG
        False
        SOURCE_DIRECTORY
        docs)

    sphinx_add_docs(
      ${CMAKE_PROJECT_NAME}-docs
      BUILDER
      html
      EXTRA_ARGS
      -T
      OUTPUT_DIRECTORY
      ${CMAKE_SOURCE_DIR}/docs/_build/html
      ${_common_sphinx_args})

    sphinx_add_docs(
      ${CMAKE_PROJECT_NAME}-docs-check
      BUILDER
      html
      EXTRA_ARGS
      $<$<BOOL:${DOCS_CHECK_NITPICKY_MODE}>:-n>
      -T
      --keep-going
      -W
      OUTPUT_DIRECTORY
      ${CMAKE_SOURCE_DIR}/docs/_build/html
      ${_common_sphinx_args})

    sphinx_add_docs(
      ${CMAKE_PROJECT_NAME}-docs-linkcheck BUILDER linkcheck OUTPUT_DIRECTORY
      ${CMAKE_SOURCE_DIR}/docs/_build/linkcheck ${_common_sphinx_args})

    unset(_common_sphinx_args)
  endif()
endif()
