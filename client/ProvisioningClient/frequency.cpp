
#include "frequency.hpp"

nlohmann::json InitializeJson(std::string input, std::default_random_engine generator, const std::time_t &time, int frequency, int totalPackets)
{
	nlohmann::json freq;
	//std::string sTime = std::to_string(1900 + ltm->tm_year) + "-" + std::to_string(ltm->tm_mon) + "-" + std::to_string(ltm->tm_mday)
	//	+ "T" + std::to_string(ltm->tm_hour) + ":" + std::to_string(ltm->tm_min) + ":" + std::to_string(ltm->tm_sec);
	
	std::uniform_int_distribution<int> distribution(900, 1000);

	freq["sensor-id"] = std::stoi(input);
	freq["frequency"] = 900 + (totalPackets*5) + frequency;
	freq["time"] = time;
	freq["reading"] = rand() % 100;
	
	return freq;
}

std::string Message(std::string input, std::default_random_engine generator, int number, const std::chrono::time_point<std::chrono::system_clock> &time, int totalPackets)
{
	std::time_t tTime = std::chrono::system_clock::to_time_t(time);
	nlohmann::json message;
	message["number"] = number;
	message["size"] = 20;
	message["time"] = tTime;
	message["lat"] = 28.599865;
	message["long"] = -81.169900;
	std::string m = message.dump();
	std::vector<nlohmann::json> freqs;
	for (int i = 0; i < 5; i++)
	{
		freqs.push_back(InitializeJson(input, generator, tTime, i, totalPackets));
	}
	nlohmann::json j_vec(freqs);	
	message["freqs"] = j_vec;

	std::string s = message.dump();
	return s;
}