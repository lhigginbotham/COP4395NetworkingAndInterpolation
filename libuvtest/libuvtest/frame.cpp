#include "frame.hpp"

FrameBuffer::FrameBuffer(std::chrono::time_point<std::chrono::system_clock> time) : recievedTime(time)
{

}


bool FrameBuffer::LifetimeExceeded()
{
	std::chrono::duration<long long, std::milli> duration(500);
	return ElapsedTime(this->recievedTime) >= duration ? true : false;
}

void FrameBuffer::Transmit(bool complete, const std::map<std::string, int> &ips, const nlohmann::json &message, uvw::UDPHandle &udp)
{
	std::chrono::time_point<std::chrono::system_clock> systemTime = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(systemTime);
	int totalSize = ips.size() * message.value("size", 0);
	std::queue<nlohmann::json> frequencies;
	for (int i = 0; i < ips.size(); i++)
	{
		for (int j = 0; j < this->sensorBuffer[i].size(); j++)
		{
			for (int k = 0; k < this->sensorBuffer[i][j]["freqs"].size(); k++)
			{
				frequencies.push(this->sensorBuffer[i][j]["freqs"][k]);
			}
		}
		this->sensorBuffer[i].resize(0);
		this->sensorBuffer[i].shrink_to_fit();
	}
	std::vector<nlohmann::json> freq;
	
	//TODO: Implement filtering here

	while (!frequencies.empty())
	{
		for (int i = 0; i < 5; i++)
		{
			if (frequencies.empty())
				break;
			freq.push_back(frequencies.front());
			frequencies.pop();
		}
		nlohmann::json alteredMessage;
		alteredMessage["complete"] = true;
		alteredMessage["freqs"] = freq;
		alteredMessage["size"] = totalSize;
		alteredMessage["time"] = currentTime;
		std::string message = alteredMessage.dump();
		//std::cout << message << "\n";
		std::string ip = globalConfig.config.value("sendip", "-1");
		unsigned int port = 4951;
		//send won't take message.c_str()
		char* data = new char[message.length() + 1];
		std::strcpy(data, message.c_str());
		unsigned int len = message.length();
		udp.send(ip, port, data, len);
		delete[] data;
	}
}

void FrameBuffer::Transmit(bool complete, const std::map<std::string, int> &ips, std::shared_ptr<uvw::UDPHandle> udp)
{
	std::chrono::time_point<std::chrono::system_clock> systemTime = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(systemTime);

	std::queue<nlohmann::json> frequencies;
	for (int i = 0; i < ips.size(); i++)
	{
		for (int j = 0; j < this->sensorBuffer[i].size(); j++)
		{
			for (int k = 0; k < this->sensorBuffer[i][j]["freqs"].size(); k++)
			{
				frequencies.push(this->sensorBuffer[i][j]["freqs"][k]);
			}
		}
		this->sensorBuffer[i].resize(0);
		this->sensorBuffer[i].shrink_to_fit();
	}

	//TODO Implement filtering

	std::vector<nlohmann::json> freq;
	int totalSize = (frequencies.size() + (5-1)) / 5;
	while (!frequencies.empty())
	{
		for (int i = 0; i < 5; i++)
		{
			if (frequencies.empty())
				break;
			freq.push_back(frequencies.front());
			frequencies.pop();
		}
		nlohmann::json alteredMessage;
		alteredMessage["complete"] = complete;
		alteredMessage["freqs"] = freq;
		alteredMessage["size"] = totalSize;
		alteredMessage["time"] = currentTime;
		std::string message = alteredMessage.dump();
		std::string ip = globalConfig.config.value("sendip", "-1");
		unsigned int port = 4951;
		//send won't take message.c_str()
		char* data = new char[message.length() + 1];
		std::strcpy(data, message.c_str());
		unsigned int len = message.length();
		udp->send(ip, port, data, len);
		delete[] data;
	}
}

std::chrono::duration<long long, std::milli> FrameBuffer::ElapsedTime(std::chrono::time_point<std::chrono::system_clock> startTime)
{
	auto endTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	return pointToMilli(endTime) - pointToMilli(startTime);
}