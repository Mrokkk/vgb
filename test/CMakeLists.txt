add_executable(test

    assembler_tests.cpp
    instruction_tests.cpp
    main.cpp
    sys/path_tests.cpp
    tools/assert.cpp
    tools/game_boy_fixture.cpp
    tools/platform_fake.cpp
    tools/platform_fake_tests.cpp
    tools/printers.cpp
    tools/watchdog.cpp

)

target_include_directories(test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(test PRIVATE emulator doctest ${LIBRARIES} assembler)
target_compile_definitions(test PRIVATE
    -DDOCTEST_CONFIG_NO_MULTITHREADING
    -DDOCTEST_CONFIG_ASSERTS_RETURN_VALUES)
