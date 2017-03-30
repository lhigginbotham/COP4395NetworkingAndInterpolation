#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <json.hpp>
#include <exception>
#include <fstream>

nlohmann::json InitializeConfig();
std::string ParseFile();

#endif 
