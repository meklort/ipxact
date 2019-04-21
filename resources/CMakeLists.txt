FUNCTION(ADD_RESOURCES out_var)
    SET(header_file ${CMAKE_BINARY_DIR}/resources/includes/resources.h)
    set(header)
    SET(result)
    FILE(WRITE ${header_file} "#ifndef RESOURCE_H\n#define RESOURCE_H\n\n")
    FILE(APPEND ${header_file} "#define RESOURCE_STRING(x) std::string(_binary_##x##_start, _binary_##x##_end - _binary_##x##_start)\n")
    FOREACH(in_f ${ARGN})
        FILE(RELATIVE_PATH src_f ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${in_f})
        get_filename_component(FILE_DIR ${src_f} DIRECTORY)
        get_filename_component(FILE_NAME ${src_f} NAME_WE)
        FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${FILE_DIR})
        # MESSAGE("Embedding file ${src_f} in ${FILE_DIR}")
        SET(out_f "${PROJECT_BINARY_DIR}/${in_f}.o")
        ADD_CUSTOM_COMMAND(OUTPUT ${out_f}
            COMMAND ld -r -b binary -o ${out_f} ${src_f}
            DEPENDS ${in_f}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Building resource object ${out_f} from ${src_f}"
            VERBATIM
        )
        SET(mangled_object ${src_f})
        STRING(REPLACE "/" "_" mangled_object ${mangled_object})
        STRING(REPLACE "." "_" mangled_object ${mangled_object})
        SET(header "${header}\nextern const char _binary_${mangled_object}_start[];\nextern const char _binary_${mangled_object}_end[];\n")
        LIST(APPEND result ${out_f})
    ENDFOREACH()
    FILE(APPEND ${header_file} "${header}\n#endif /* !RESOURCE_H */")
    SET(${out_var} "${result}" PARENT_SCOPE)
ENDFUNCTION()
