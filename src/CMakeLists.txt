add_library(libsodb process.cpp pipe.cpp)
add_library(sodb::libsodb ALIAS libsodb)

set_target_properties(
    libsodb
    PROPERTIES OUTPUT_NAME sodb
)

target_compile_features(libsodb PUBLIC cxx_std_23)

target_include_directories(libsodb
    PUBLIC 
        $<INSTALL_INTERFACE::include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/include
)

include(GNUInstallDirs)
install(TARGETS libsodb
    EXPORT   sodb-targets
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY   ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT      sodb-targets
    FILE        sodb-config.cmake
    NAMESPACE   sodb::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sodb
)
