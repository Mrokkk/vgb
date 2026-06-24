add_executable(test
    instruction_tests.cpp
    main.cpp
)

target_link_libraries(test PUBLIC emulator doctest ${LIBRARIES})
