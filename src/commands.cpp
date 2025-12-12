#include <iostream>

#include "console.hpp"

int main() 
{
		auto echoNode = std::make_unique<Node>();
    echoNode->function = [](const std::vector<std::string>& args){
        for (auto& s : args) std::cout << s << " ";
        std::cout << "\n";
    };
		auto echoNode2 = std::make_unique<Node>();
    echoNode->function = [](const std::vector<std::string>& args){
        for (auto& s : args) std::cout << s << " ";
        std::cout << "\n";
    };

    auto stopNode = std::make_unique<Node>();
    stopNode->function = [&](const std::vector<std::string>&){
        std::cout << "Stopping console\n";
    };

    Console console;
    console.nodes["stop"] = std::move(stopNode);
    console.nodes["abcdefg"] = std::move(echoNode);
    console.nodes["abcdfeg"] = std::move(echoNode2);
	runConsole(console);
	
}
