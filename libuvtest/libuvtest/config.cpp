#include "config.hpp"

nlohmann::json InitializeConfig()
{
	nlohmann::json config;
	std::string configSerial = ParseFile();



	return config;
}

std::string ParseFile()
{
	std::ifstream configFile;
	std::string config;


	configFile.open("config.json", std::fstream::in);
	configFile >> config;
		
	return config;
}