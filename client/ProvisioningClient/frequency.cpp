
#include "frequency.hpp"

nlohmann::json InitializeJson(std::string input, std::default_random_engine generator)
{
	nlohmann::json freq;
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t); //localtime is not thread safe
	std::string time = "2017-12-03T12:00:00";// + tm.tm_mon; + "-" + tm.tm_mday + "T" + tm.tm_hour + ":" + tm.tm_min + ":" + tm.tm_sec;
	std::uniform_int_distribution<int> distribution(900, 1000);

	freq["sensor-id"] = input;
	freq["frequency"] = distribution(generator);
	freq["time"] = time;
	freq["reading"] = -75;
	
	return freq;
}

nlohmann::json Message(std::string input, std::default_random_engine generator)
{
	nlohmann::json message;
	std::vector<nlohmann::json> freqs;
	for (int i = 0; i < 30; i++)
	{
		freqs.push_back(InitializeJson(input, generator));
	}
	nlohmann::json j_vec(freqs);
	message["number"] = 1;
	message["freqs"] = j_vec;

	return message;
}