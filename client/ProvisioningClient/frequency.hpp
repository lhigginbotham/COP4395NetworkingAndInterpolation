#ifndef FREQUENCY_HPP_
#define FREQUENCY_HPP_

#define _CRT_SECURE_NO_WARNINGS

#include <json.hpp>
#include <iomanip>
#include <ctime>
#include <random>

nlohmann::json InitializeJson(std::string input, std::default_random_engine generator);
std::string Message(std::string input, std::default_random_engine generator, int number);

#endif