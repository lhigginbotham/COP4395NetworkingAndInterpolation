
#include "frequency.hpp"

nlohmann::json InitializeJson(std::string input, std::default_random_engine generator, const std::time_t &time)
{
	nlohmann::json freq;
	tm *ltm = std::localtime(&time); //localtime is not thread safe
	std::string sTime = std::to_string(1900 + ltm->tm_year) + "-" + std::to_string(ltm->tm_mon) + "-" + std::to_string(ltm->tm_mday)
		+ "T" + std::to_string(ltm->tm_hour) + ":" + std::to_string(ltm->tm_min) + ":" + std::to_string(ltm->tm_sec);
	
	std::uniform_int_distribution<int> distribution(900, 1000);

	freq["sensor-id"] = input;
	freq["frequency"] = 900 + rand() % 100;
	freq["time"] = sTime;
	freq["reading"] = -75;
	
	return freq;
}

std::string Message(std::string input, std::default_random_engine generator, int number, const std::chrono::time_point<std::chrono::system_clock> &time)
{
	std::time_t tTime = std::chrono::system_clock::to_time_t(time);
	nlohmann::json message;
	message["number"] = number;
	message["size"] = 3;
	std::string m = message.dump();
	std::vector<nlohmann::json> freqs;
	for (int i = 0; i < 5; i++)
	{
		freqs.push_back(InitializeJson(input, generator, tTime));
	}
	nlohmann::json j_vec(freqs);	
	message["freqs"] = j_vec;

	std::string s = message.dump();
	return s;
}