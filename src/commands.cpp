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

		console["set"]["newLineAutoComplete"]["1"].function = console["set"]["newLineAutoComplete"]["true"].function =
				[&](const std::vector<std::string>&)
				{
					console.settings.newLineAutoComplete = true;
					std::cout<<"newLineAutoComplete was set to true"<<std::endl;
				};
		
		console["set"]["newLineAutoComplete"]["0"].function = console["set"]["newLineAutoComplete"]["false"].function =
				[&](const std::vector<std::string>&)
				{
					console.settings.newLineAutoComplete = false;
					std::cout<<"newLineAutoComplete was set to false"<<std::endl;
				};

		console["exit"].function =
				[&](const std::vector<std::string>&)
				{
					console.stop = true;
				};
	runConsole(console);
	
}
