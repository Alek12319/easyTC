project(easyExample)

set(SRC_CPP test.cpp)
add_executable(${PROJECT_NAME} ${SRC_CPP})
target_link_libraries(${PROJECT_NAME} easyTCTool)
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
)