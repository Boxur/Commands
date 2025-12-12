#include <iostream>

#include "console.hpp"

int main() 
{
    Console console;

		console["ping"].function =
        [&](const std::vector<std::string>&)
        {
					std::cout << "pong\n";
        };

    console["set"]["mode"]["automat"].function =
        [&](const std::vector<std::string>&)
        {
					std::cout << "Mode set to AUTO.\n";
        };

		console["set"]["mode"]["automobile"].function = 
				[&](const std::vector<std::string>&)
				{
					return;
				};

		console["exit"].function =
				[&](const std::vector<std::string>&)
				{
					console.stop = true;
				};
	runConsole(console);
	
}
