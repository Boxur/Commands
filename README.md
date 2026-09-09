# CLI

An easy to use c++ library allowing for easy creation of custom commands
executable at runtime.

[![Build](https://github.com/Boxur/CLI/actions/workflows/documentation.yml/badge.svg)](https://github.com/boxur/CLI/actions)
[![Documentation](https://img.shields.io/badge/docs-online-blue)](https://cli.documentation.boxur.org/)
[![License](https://img.shields.io/github/license/boxur/CLI)](LICENSE)
[![Top Language](https://img.shields.io/github/languages/top/boxur/NeuralNetwork)](https://github.com/boxur/NeuralNetwork)

## Features

 - Creating custom commands
 - Running custom commands with user input at runtime
 - Running custom command scripts from files

## Requirements

 - C++23 compiler
 - CMake 3.14+
 - Git

## Instalation

Clone from github:
```bash
git clone https://github.com/Boxur/CLI external/cli
```

### CMake

Add to your project in CMake
```cmake
add_subdirectory(external/cli)

target_link_libraries(project PRIVATE cli)
```

## Examples

Minimal running example:

```cpp
#include <cli/console.hpp>

int main(){
  cli::Console console;
  console.Run();
}
```

Full use example can be found in [src/commands.cpp](src/commands.cpp)

## Contributing

Contributions, bug reports, and suggestions are welcome.

Before submitting a pull request, please ensure that the project builds
successfully

## License

This project is licensed under the MIT License.
See [LICENSE](LICENSE) for details.
