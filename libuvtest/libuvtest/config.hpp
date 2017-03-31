#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <json.hpp>
#include <exception>
#include <fstream>

class ConfigStore
{
public:
	ConfigStore(std::string fileName);
	~ConfigStore() = default;
	nlohmann::json config;
	int getType();

private:
	std::string ParseFile(std::string fileName);
	bool ValidateConfig(const nlohmann::json &config);
	//Denote the type of server in operation: 0 = provisioning, 1 = handling
	int type;
};
#endif 