#ifndef UTILITIES_HPP_
#define UTILITIES_HPP_

#include <chrono>


std::chrono::duration<long long, std::milli> pointToMilli(std::chrono::time_point<std::chrono::system_clock> point);

#endif // !UTILITIES_HPP_