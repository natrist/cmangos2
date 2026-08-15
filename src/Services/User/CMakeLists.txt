set(LIBRARY_NAME user)

add_library(${LIBRARY_NAME} STATIC
    UserClient.cpp
    UserService.cpp
)

target_link_libraries(${LIBRARY_NAME}
    PUBLIC game
    PUBLIC shared
    PUBLIC g3dlite
    cmangos-compile-option-interface
)

target_include_directories(${LIBRARY_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC)
    set_target_properties(${LIBRARY_NAME} PROPERTIES PROJECT_LABEL "UserService")
endif()