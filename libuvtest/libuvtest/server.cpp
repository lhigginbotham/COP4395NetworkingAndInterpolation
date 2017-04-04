#define _CRT_SECURE_NO_WARNINGS

#include <deque>
#include <iostream>
#include <memory>
#include <json.hpp>
#include <uvw.hpp>

#include "config.hpp"
#include "frame.hpp"

//static std::vector<std::vector<std::vector<nlohmann::json>>> freqBuffer (10, std::vector<std::vector<nlohmann::json>>(0, std::vector<nlohmann::json> (0)));
static std::deque<FrameBuffer> frameBuffer;
static const ConfigStore globalConfig ("config.log");

void listen(uvw::Loop &loop, std::map<std::string, int> &ips) {
	std::shared_ptr<uvw::UDPHandle> udp = loop.resource<uvw::UDPHandle>();
	udp->bind("127.0.0.1", 4951);
	udp->recv();
	int cPosition = 0;

	udp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &err, uvw::UDPHandle &) {
		std::cout << "Code: " << err.code() << " Message: " << err.what() << "\n";
	});

	udp->on<uvw::UDPDataEvent>([&ips, cPosition](const uvw::UDPDataEvent &sData, uvw::UDPHandle &udp) {
		std::string result = sData.data.get();
		bool transmitBuffer = false;
		//Trim off excess data transmitted from client
		std::string complete = result.substr(0, sData.length);
		//So, this is bizarre.  Intellisense flags this as an error when passing a standard std::string but it compiles and runs regardless
		//Converting it to a Cstring causes Intellisense to no longer flag it
		//Unsure what a proper fix to this would be as library dev blames it on Intellisense (and the fact that it compiles and runs regardless supports that)
		nlohmann::json freq = nlohmann::json::parse(complete.c_str());
		//std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();

		if (frameBuffer.empty())
		{
			time_t recievedTime = freq.value("time", 0);
			frameBuffer.push_back(FrameBuffer(std::chrono::system_clock::from_time_t(recievedTime)));
		}

		if (ips.find(sData.sender.ip) == ips.end())
		{
			ips.emplace(sData.sender.ip, ips.size());
			frameBuffer[cPosition].sensorBuffer.push_back(std::vector<nlohmann::json>());
		}
		if (frameBuffer[cPosition].sensorBuffer.size() != ips.size())
		{
			frameBuffer[cPosition].sensorBuffer.resize(ips.size());//This may break existing iterators so watch out for that
		}

		int ipPosition = ips.find(sData.sender.ip)->second;
		auto front = frameBuffer.begin();
		time_t freqTime = freq.value("time", 0);
		time_t duration = 50;
		std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		time_t lowerBound = std::chrono::system_clock::to_time_t(front->recievedTime);
		time_t upperBound = std::chrono::system_clock::to_time_t(front->recievedTime) + duration;
		if (freqTime < std::chrono::system_clock::to_time_t(front->recievedTime))
		{
			return;
		}
		else if (freqTime >= std::chrono::system_clock::to_time_t(front->recievedTime) && 
			freqTime < (std::chrono::system_clock::to_time_t(front->recievedTime) + duration))
		{
			front->sensorBuffer[ipPosition].push_back(freq);
		}
		else
		{
			for (auto &i : frameBuffer)
			{
				if(freqTime >= std::chrono::system_clock::to_time_t(front->recievedTime) &&
					freqTime < (std::chrono::system_clock::to_time_t(front->recievedTime) + duration))
				{
					i.sensorBuffer[ipPosition].push_back(freq);
					break;
				}
			}
			frameBuffer.push_back(FrameBuffer(std::chrono::system_clock::from_time_t(freq.value("time", 0))));
			frameBuffer[cPosition].sensorBuffer.resize(ips.size());//This may break existing iterators so watch out for that
			frameBuffer.end()->sensorBuffer[ipPosition].push_back(freq);
		}

		int vFullTracker = 0;
		if (frameBuffer[cPosition].sensorBuffer[ipPosition].size() >= ips.size())
		{
			for (auto &&i : frameBuffer[cPosition].sensorBuffer)
			{
				if (i.size() >= freq.value("size", 0))
				{
					vFullTracker++;
				}
			}
			if (vFullTracker >= ips.size())
			{
				transmitBuffer = true;
			}
		}
		if (transmitBuffer)
		{
			for (int i = 0; i < ips.size(); i++)
			{
				for (int j = 0; j < freq.value("size", 0); j++)
				{
					std::string message = frameBuffer[cPosition].sensorBuffer[i][j].dump();
					uvw::Addr addr;
					std::string ip = globalConfig.config.value("sendip", "-1");
					unsigned int port = 4951;
					//send won't take message.c_str()
					char* data = new char[message.length() + 1];
					std::strcpy(data, message.c_str());
					unsigned int len = message.length();
					udp.send(ip, port, data, len);
					delete[] data;
				}
				frameBuffer[cPosition].sensorBuffer[i].resize(0);
				frameBuffer[cPosition].sensorBuffer[i].shrink_to_fit();
			}
			frameBuffer.pop_front();
		}

		std::cout << "Length: " << sData.length << " Sender: " << sData.sender.ip << " Data: " << complete << "\n";
	});
}

void timer(uvw::Loop &loop)
{
	auto timer = loop.resource<uvw::TimerHandle>();
	std::chrono::milliseconds duration(2000);
	timer->start(duration, duration);
	std::shared_ptr<uvw::UDPHandle> udp = loop.resource<uvw::UDPHandle>();
	
	timer->on<uvw::TimerEvent>([&udp](const uvw::TimerEvent &, uvw::TimerHandle &timer) {
		std::cout << "In timer \n";
	});
	
}

int main() {
	auto loop = uvw::Loop::getDefault();
	std::map<std::string, int> ips;
	listen(*loop, ips);
	timer(*loop);
	loop->run();
	return 0;
}