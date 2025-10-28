add_executable(sodb sodb.cpp)
target_link_libraries(sodb PRIVATE sodb::libsodb PkgConfig::libedit)

include(GNUInstallDirs)
install(
    TARGETS sodb
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
