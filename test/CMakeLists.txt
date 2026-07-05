add_executable(test

    instruction_tests.cpp
    main.cpp
    sys/path_tests.cpp
    tools/assert.cpp
    tools/platform_fake.cpp
    tools/platform_fake_tests.cpp

)

target_include_directories(test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(test PUBLIC emulator doctest ${LIBRARIES})
target_compile_definitions(test PRIVATE
    -DDOCTEST_CONFIG_NO_MULTITHREADING
    -DDOCTEST_CONFIG_ASSERTS_RETURN_VALUES)
