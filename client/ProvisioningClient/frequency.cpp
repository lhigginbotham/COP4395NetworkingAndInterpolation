
#include "frequency.hpp"

nlohmann::json InitializeJson(std::string input)
{
	nlohmann::json freq;
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t); //localtime is not thread safe
	std::string time = "2017-12-03T12:00:00";// + tm.tm_mon; + "-" + tm.tm_mday + "T" + tm.tm_hour + ":" + tm.tm_min + ":" + tm.tm_sec;
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(900, 1000);

	freq["sensor-id"] = input;
	freq["frequency"] = distribution(generator);
	freq["time"] = time;
	freq["reading"] = -75;
	
	return freq;
}