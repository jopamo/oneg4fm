cmake_minimum_required(VERSION 3.18)

# Find Qt6 Test components
find_package(Qt6 REQUIRED COMPONENTS Test)

# Create test executable
add_executable(pcmanfm-qt-tests
    test_basic.cpp
)

# Link with Qt Test
target_link_libraries(pcmanfm-qt-tests
    Qt6::Test
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Include directories
target_include_directories(pcmanfm-qt-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/pcmanfm
)

# Add test
add_test(NAME pcmanfm-qt-tests COMMAND pcmanfm-qt-tests)