#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <json.hpp>
#include <exception>
#include <fstream>

class ConfigStore
{
public:
	ConfigStore(const std::string fileName);
	~ConfigStore() = default;
	nlohmann::json config;
	int type;

private:
	std::string ParseFile(const std::string fileName);
	bool ValidateConfig(const nlohmann::json &config);
	//Denote the type of server in operation: 0 = provisioning, 1 = handling
};

extern const ConfigStore globalConfig;
#endif 
