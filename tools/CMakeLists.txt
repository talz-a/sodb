add_executable(sodb sodb.cpp)
target_link_libraries(sodb PRIVATE sodb::libsodb)

include(GNUInstallDirs)
install(
    TARGETS sodb
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
