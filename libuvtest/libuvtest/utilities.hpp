#ifndef UTILITIES_HPP_
#define UTILITIES_HPP_

#include <chrono>
#include <ctime>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>

#include "config.hpp"

bool addSensorDatabase(const nlohmann::json &message);
std::chrono::duration<long long, std::milli> pointToMilli(std::chrono::time_point<std::chrono::system_clock> point);
std::chrono::duration<long long, std::milli> time_tToMilli(std::time_t time);

#endif // !UTILITIES_HPP_