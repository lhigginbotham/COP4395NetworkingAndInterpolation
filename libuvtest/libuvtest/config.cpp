#include "config.hpp"

ConfigStore::ConfigStore(const std::string fileName)
{
	std::string configSerial = ParseFile(fileName);

	config = nlohmann::json::parse(configSerial.c_str());

	if (!ValidateConfig(config))
	{
		std::cout << "Failed to parse configuration\n";
		exit(-1);
	}
}


std::string ConfigStore::ParseFile(const std::string fileName)
{
	std::ifstream configFile;
	std::string config;

	configFile.open(fileName, std::fstream::in);
	if (configFile.is_open())
	{
		configFile.seekg(0, std::ios::end);
		config.resize(configFile.tellg());
		configFile.seekg(0, std::ios::beg);
		configFile.read(&config[0], config.size());
		configFile.close();
		return config;
	}
	std::cout << "Failed to open file: " << fileName << "\n";
	exit(-1);
}

bool ConfigStore::ValidateConfig(const nlohmann::json &config)
{
	bool valid = true;
	int tempValue;
	type = config.value("type", -1);
	switch (type) {
		case 0:
			if (config.value("sendip", "-1").compare("-1") == 0)
			{
				valid = false;
				break;
			}
			if (config.value("interval", -1) < 0 && config.value("freqlow", -1) < 0 && config.value("freqhigh", -1))
			{
				valid = false;
				break;
			}

			break;
		case 1:

		default:
			valid = false;
	}
	return valid;
}

int ConfigStore::getType()
{
	return type;
}