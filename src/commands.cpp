#include <iostream>
#include <algorithm>

#include "console.hpp"

int main(int argc,char* argv[]) 
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

	console["set"]["newLineAutoComplete"]["show"].function =
		[&](const std::vector<std::string>&)
		{
			std::cout<<"newLineAutoComplete is set to "<<((console.settings.newLineAutoComplete) ? "true" : "false")<<std::endl;
		};
	
	console["set"]["histSize"].function =
		[&](const std::vector<std::string>& args)
		{
			if(args.size()<1||!all_of(args[0].begin(),args[0].end(),::isdigit))
			{
				std::cout<<"Usage: set histSize [integer]";
				return;
			}
			console.settings.histSize = stoi(args[0]);
			std::cout<<"histSize was set to "<<console.settings.histSize<<std::endl;

		};

	console["exit"].function =
		[&](const std::vector<std::string>&)
		{
			console.stop = true;
		};

	if(argc==1)
		runConsole(console);
	else
		runConsole(console,argv[1]);
	
}
