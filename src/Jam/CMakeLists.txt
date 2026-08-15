#
# Jam types/runtime helpers.
# Definitions + JamMessage: src/JamAutoCode/
# Generated protocol code:  ${CMAKE_BINARY_DIR}/src/JamAutoCode/
#

set(LIBRARY_NAME jam)

add_library(${LIBRARY_NAME} STATIC
    JamTypes.h
    JamDynamicString.h
    JamDynamicString.cpp
    JamDynamicArray.h
    ${CMAKE_SOURCE_DIR}/src/JamAutoCode/JamMessage.h
    ${CMAKE_SOURCE_DIR}/src/JamAutoCode/JamMessage.cpp
)

# "Jam/...", "JamAutoCode/JamMessage.h", "Services/WowConnection.h"
# Generated: "JamAutoCode/JamUserClient.h" from ${CMAKE_BINARY_DIR}/src
target_include_directories(${LIBRARY_NAME}
    PUBLIC ${CMAKE_SOURCE_DIR}/src
    PUBLIC ${CMAKE_BINARY_DIR}/src
)

target_link_libraries(${LIBRARY_NAME}
    PUBLIC shared
    PUBLIC g3dlite
    cmangos-compile-option-interface
)

# --- jamgen: src/JamAutoCode/*.jam → ${CMAKE_BINARY_DIR}/src/JamAutoCode/ ---

if(NOT TARGET JamGen)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/tools/JamGen ${CMAKE_BINARY_DIR}/tools/JamGen)
endif()

set(JAM_DEFINITIONS_DIR "${CMAKE_SOURCE_DIR}/src/JamAutoCode")
set(JAM_GENERATED_DIR   "${CMAKE_BINARY_DIR}/src/JamAutoCode")
file(MAKE_DIRECTORY "${JAM_GENERATED_DIR}")

file(GLOB JAM_DEFINITIONS CONFIGURE_DEPENDS "${JAM_DEFINITIONS_DIR}/*.jam")

set(JAM_GENERATED_OUTPUTS "")
foreach(JAM_FILE ${JAM_DEFINITIONS})
    get_filename_component(JAM_BASE ${JAM_FILE} NAME_WE)
    set(GEN_BASE "Jam${JAM_BASE}")
    set(GEN_H   "${JAM_GENERATED_DIR}/${GEN_BASE}.h")
    set(GEN_CPP "${JAM_GENERATED_DIR}/${GEN_BASE}.cpp")

    add_custom_command(
        OUTPUT  ${GEN_H} ${GEN_CPP}
        COMMAND $<TARGET_FILE:JamGen> ${JAM_FILE} ${JAM_GENERATED_DIR}
        DEPENDS JamGen ${JAM_FILE}
        COMMENT "Generating ${GEN_BASE} -> build/.../JamAutoCode"
        VERBATIM
    )

    list(APPEND JAM_GENERATED_OUTPUTS ${GEN_H} ${GEN_CPP})
endforeach()

set_source_files_properties(${JAM_GENERATED_OUTPUTS} PROPERTIES GENERATED TRUE)
add_custom_target(JamGenAll DEPENDS ${JAM_GENERATED_OUTPUTS})
add_dependencies(${LIBRARY_NAME} JamGenAll)

set_source_files_properties(${JAM_DEFINITIONS} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group("Definitions" FILES ${JAM_DEFINITIONS})
source_group("JamAutoCode" FILES
    ${CMAKE_SOURCE_DIR}/src/JamAutoCode/JamMessage.h
    ${CMAKE_SOURCE_DIR}/src/JamAutoCode/JamMessage.cpp
)
source_group("Generated" FILES ${JAM_GENERATED_OUTPUTS})

set(JAM_GENERATED_FILES ${JAM_GENERATED_OUTPUTS} PARENT_SCOPE)
set(JAM_GENERATED_DIR ${JAM_GENERATED_DIR} PARENT_SCOPE)

if(MSVC)
    set_target_properties(${LIBRARY_NAME} PROPERTIES PROJECT_LABEL "Jam")
endif()
