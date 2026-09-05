#include <iostream>

#include "console.hpp"

int main(int argc, char *argv[]) {
  cli::Console console;

  console["ping"].function = [&](const std::vector<std::string> &) {
    std::cout << "pong\n";
  };

  if (argc == 1)
    console.Run();
  else
    console.Run(argv[1]);
}
