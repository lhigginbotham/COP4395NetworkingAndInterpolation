
#include "frequency.hpp"

nlohmann::json InitializeJson(std::string input, std::default_random_engine generator)
{
	nlohmann::json freq;
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t); //localtime is not thread safe
	std::string time = "2017-12-03T12:00:00";// + tm.tm_mon; + "-" + tm.tm_mday + "T" + tm.tm_hour + ":" + tm.tm_min + ":" + tm.tm_sec;
	std::uniform_int_distribution<int> distribution(900, 1000);

	freq["sensor-id"] = input;
	freq["frequency"] = 900 + rand() % 100;
	freq["time"] = time;
	freq["reading"] = -75;
	
	return freq;
}

std::string Message(std::string input, std::default_random_engine generator, int number)
{
	nlohmann::json message;
	message["number"] = number;
	message["size"] = 3;
	std::string m = message.dump();
	std::vector<nlohmann::json> freqs;
	for (int i = 0; i < 5; i++)
	{
		freqs.push_back(InitializeJson(input, generator));
	}
	nlohmann::json j_vec(freqs);	
	message["freqs"] = j_vec;

	std::string s = message.dump();
	return s;
}