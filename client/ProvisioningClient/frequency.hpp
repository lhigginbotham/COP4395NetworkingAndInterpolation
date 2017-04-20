#ifndef FREQUENCY_HPP_
#define FREQUENCY_HPP_

#define _CRT_SECURE_NO_WARNINGS

#include <json.hpp>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <random>

nlohmann::json InitializeJson(std::string input, std::default_random_engine generator, const std::time_t &time, int frequency, int totalPackets);
std::string Message(std::string input, std::default_random_engine generator, int number, const std::time_t time, int totalPackets);

#endif