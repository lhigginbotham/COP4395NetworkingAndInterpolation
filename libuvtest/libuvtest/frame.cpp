#include "frame.hpp"

FrameBuffer::FrameBuffer(std::chrono::time_point<std::chrono::system_clock> time) : recievedTime(time)
{

}


bool FrameBuffer::LifetimeExceeded() 
{
	std::chrono::duration<long long, std::milli> duration(500);
	return ElapsedTime(this->recievedTime) >= duration ? true : false;
}

void FrameBuffer::Transmit(bool complete, const std::map<std::string, int> &ips, const nlohmann::json &freq, uvw::UDPHandle &udp)
{
	for (int i = 0; i < ips.size(); i++)
	{
		for (int j = 0; j < freq.value("size", 0); j++)
		{
			std::string message = this->sensorBuffer[i][j].dump();
			std::string ip = globalConfig.config.value("sendip", "-1");
			unsigned int port = 4951;
			//send won't take message.c_str()
			char* data = new char[message.length() + 1];
			std::strcpy(data, message.c_str());
			unsigned int len = message.length();
			udp.send(ip, port, data, len);
			delete[] data;
		}
		this->sensorBuffer[i].resize(0);
		this->sensorBuffer[i].shrink_to_fit();
	}
}

std::chrono::duration<long long, std::milli> FrameBuffer::ElapsedTime(std::chrono::time_point<std::chrono::system_clock> startTime)
{
	auto endTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	return pointToMilli(endTime) - pointToMilli(startTime);
}