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

    // Command 2: "set mode auto" (recursive predefined arguments)
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
		std::cout<<"-"<<(console["set"].function==nullptr)<<"-"<<std::endl;
	runConsole(console);
	
}
