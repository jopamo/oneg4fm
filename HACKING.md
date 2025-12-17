# Developer Guide (HACKING)

This guide provides instructions for building, testing, and contributing to pcmanfm-qt.

For architectural details, please refer to [DESIGN.md](DESIGN.md).

## Development Environment

### Dependencies
To build pcmanfm-qt, you need the following dependencies:
- CMake >= 3.18.0
- Qt 6 >= 6.6.0 (Core, DBus, LinguistTools, Widgets, Concurrent)
- libfm-qt >= 2.3.0
- libarchive
- blake3
- capstone
- pkg-config
- Doxygen (optional, for documentation)

### Building
In-source builds are not supported. Please use a separate build directory.

```bash
mkdir build
cd build
cmake ..
make
```

### Running
After building, you can run the executable directly from the build directory:

```bash
./pcmanfm/pcmanfm-qt
```

### Testing
To run the test suite, you must configure the build with `BUILD_TESTING=ON`.

```bash
cd build
cmake -DBUILD_TESTING=ON ..
make
ctest
```

## Coding Guidelines

### Language
The project uses C++ (compatible with Qt 6).

### Style
This project uses `clang-format` for code formatting. Please ensure your changes comply with the rules defined in the `.clang-format` file in the project root.

You can format your code using:
```bash
clang-format -i path/to/source/file.cpp
```

## Contribution Flow

### Proposing Changes
1. Fork the repository.
2. Create a feature branch.
3. Make your changes, ensuring they build and pass tests.
4. Submit a pull request.

### Before Submitting
- Run `clang-format` on your changed files.
- Ensure the project builds successfully.
- Run the tests using `ctest` to catch regressions.