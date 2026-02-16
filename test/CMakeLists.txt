add_subdirectory("targets")
add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE sodb::libsodb Catch2::Catch2WithMain)

target_compile_definitions(tests PRIVATE
    RUN_ENDLESSLY_PATH="$<TARGET_FILE:run_endlessly>"
)
target_compile_definitions(tests PRIVATE
    END_IMMEDIATELY_PATH="$<TARGET_FILE:end_immediately>"
)

target_compile_definitions(tests PRIVATE
    REG_WRITE_PATH="$<TARGET_FILE:reg_write>"
)
