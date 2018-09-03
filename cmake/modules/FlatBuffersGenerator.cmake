function(GENERATE_FLATBUFFERS_HEADER HDRS)
    cmake_parse_arguments(ARG "DEBUG" "FLATBUFFERROOT;NAME;OUTPATH;EXPORT_MACRO;TARGET" "FLATBUFFERFILE" ${ARGN})

    if(NOT ARG_FLATBUFFERFILE)
        message(SEND_ERROR "Error: GENERATE_FLATBUFFERS_HEADER() called without any flatbuffers files")
    return()
endif(NOT ARG_FLATBUFFERFILE)

  if(NOT ARG_NAME)
      message(SEND_ERROR "Error: GENERATE_FLATBUFFERS_HEADER() called without any class name")
    return()
  endif(NOT ARG_NAME)

  list(LENGTH ARG_FLATBUFFERSROOT FLATBUFFERS_LENGTH)
  if(FLATBUFFERSROOT_LENGTH GREATER 1)
      message(SEND_ERROR "Error: GENERATE_FLATBUFFERS_HEADER() called with too many flatbuffersroots, only one is allowed")
    return()
  endif()

  list(LENGTH ARG_OUTPATH OUTPATH_LENGTH)
  if(OUTPATH_LENGTH GREATER 1)
      message(SEND_ERROR "Error: GENERATE_FLATBUFFERS_HEADER() called with too many outpaths, only one is allowed")
    return()
  endif()

  set(OUTPATH ${CMAKE_CURRENT_BINARY_DIR})
  if(OUTPATH_LENGTH EQUAL 1)
    set(OUTPATH ${ARG_OUTPATH})
  endif()

  set(FLATBUFFERSROOT ${CMAKE_CURRENT_SOURCE_DIR})
  if(FLATBUFFERSOOT_LENGTH EQUAL 1)
      set(FLATBUFFERSROOT ${ARG_FLATBUFFERSROOT})
  endif()

  if(ARG_DEBUG)
    message("OUTPATH: ${OUTPATH}")
    message("FLATBUFFERSFILE: ${ARG_FLATBUFFERSFILE}")
  endif()

  set(${HDRS})
  foreach(FLATBUFFERSFILE ${ARG_FLATBUFFERSFILE})

    # ensure that the file ends with .proto
    string(REGEX MATCH "\\.flatc$$" PROTOEND ${FLATBUFFERSFILE})
    if(NOT PROTOEND)
        message(SEND_ERROR "Flatbuffers file '${FLATBUFFERSFILE}' does not end with .flatc")
    endif()

    get_filename_component(FLATBUFFERS_PATH ${FLATBUFFERSFILE} PATH)
    get_filename_component(ABS_FILE ${FLATBUFFERSFILE} ABSOLUTE)
    get_filename_component(FILE_WE ${FLATBUFFERSFILE} NAME_WE)

    if(ARG_DEBUG)
        message("file ${FLATBUFFERSFILE}:")
        message("  PATH=${FLATBUFFERS_PATH}")
      message("  ABS_FILE=${ABS_FILE}")
      message("  FILE_WE=${FILE_WE}")
      message("  FLATBUFFERSROOT=${FLATBUFFERSROOT}")
    endif()

    # find out of the file is in the specified proto root
    # TODO clean the SQLROOT so that it does not form a regex itself?
    string(REGEX MATCH "^${FLATBUFFERSROOT}" IN_ROOT_PATH ${FLATBUFFERSFILE})
    string(REGEX MATCH "^${FLATBUFFERSROOT}" IN_ROOT_ABS_FILE ${ABS_FILE})

    if(IN_ROOT_PATH)
        set(MATCH_PATH ${FLATBUFFERSFILE})
    elseif(IN_ROOT_ABS_FILE)
      set(MATCH_PATH ${ABS_FILE})
    else()
        message(SEND_ERROR "Flatbuffers file '${FLATBUFFERSFILE}' is not in flatbuffersroot '${FLATBUFFERSROOT}'")
    endif()

    # build the result file name
    string(REGEX REPLACE "^${FLATBUFFERSROOT}(/?)" "" ROOT_CLEANED_FILE ${MATCH_PATH})
    if(ARG_DEBUG)
      message("  ROOT_CLEANED_FILE=${ROOT_CLEANED_FILE}")
    endif()
    string(REGEX REPLACE "\\.flatc$$" "" EXT_CLEANED_FILE ${ROOT_CLEANED_FILE})
    if(ARG_DEBUG)
        message("  EXT_CLEANED_FILE=${EXT_CLEANED_FILE}")
    endif()

    set(H_FILE "${OUTPATH}/${ARG_NAME}.h")

    if(ARG_DEBUG)
      message("  H_FILE=${H_FILE}")
    endif()

    list(APPEND ${HDRS} "${H_FILE}")

    add_custom_command(
      OUTPUT "${H_FILE}"
      COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPATH}
      COMMAND flatc
      ARGS ${MATCH_PATH} "${OUTPATH}/${ARG_NAME}" ${ARG_NAME}
      DEPENDS ${ABS_FILE}
      COMMENT "Running flatc compiler on ${MATCH_PATH} with root ${FLATBUFFERSROOT}, generating: ${H_FILE}"
      VERBATIM)

  endforeach()
  
  add_custom_target(${ARG_NAME}_generated
    DEPENDS ${${HDRS}}
  )

  set_source_files_properties(${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)

endfunction()
