
#include "utilities.hpp"

std::chrono::duration<long long, std::milli> pointToMilli(std::chrono::time_point<std::chrono::system_clock> point)
{
	auto nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(point);
	std::chrono::milliseconds epoch = nowMs.time_since_epoch();
	auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
	int duration = value.count();
	std::chrono::duration<long long, std::milli> mTime(duration);
	return mTime;
}

std::chrono::duration<long long, std::milli> time_tToMilli(std::time_t time)
{
	std::chrono::system_clock::time_point nPoint = std::chrono::system_clock::from_time_t(time);
	return pointToMilli(nPoint);
}