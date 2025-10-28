add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE sodb::libsodb Catch2::Catch2WithMain)
