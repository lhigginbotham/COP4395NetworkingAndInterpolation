#define _CRT_SECURE_NO_WARNINGS

#include <deque>
#include <iostream>
#include <json.hpp>
#include <uvw.hpp>

#include "config.hpp"
#include "frame.hpp"

//static std::vector<std::vector<std::vector<nlohmann::json>>> freqBuffer (10, std::vector<std::vector<nlohmann::json>>(0, std::vector<nlohmann::json> (0)));
static std::deque<FrameBuffer> frameBuffer;
const ConfigStore globalConfig ("config.log");

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
			frameBuffer.begin()->sensorBuffer.push_back(std::vector<nlohmann::json>());
		}
		if (frameBuffer[cPosition].sensorBuffer.size() != ips.size())
		{
			frameBuffer[cPosition].sensorBuffer.resize(ips.size());//This may break existing iterators so watch out for that
		}

		int ipPosition = ips.find(sData.sender.ip)->second;
		auto front = frameBuffer.begin();
		std::chrono::milliseconds duration(500);
		std::chrono::milliseconds freqTime = time_tToMilli(freq.value("time", 0));
		std::chrono::milliseconds lowerBound = pointToMilli(front->recievedTime);
		std::chrono::milliseconds upperBound = lowerBound + duration;
		if (freqTime < lowerBound)
		{
			return;
		}
		else if (freqTime >= lowerBound && (freqTime < upperBound))
		{
			front->sensorBuffer[ipPosition].push_back(freq);
		}
		else
		{
			for (auto &i : frameBuffer)
			{
				if(freqTime >= pointToMilli(i.recievedTime) && (freqTime < pointToMilli(i.recievedTime) + duration))
				{
					i.sensorBuffer[ipPosition].push_back(freq);
					break;
				}
			}
			frameBuffer.push_back(FrameBuffer(std::chrono::system_clock::from_time_t(freq.value("time", 0))));
			//Using end() here causes a dereference error for some reason
			frameBuffer.back().sensorBuffer.push_back(std::vector<nlohmann::json>());
			frameBuffer.back().sensorBuffer.resize(ips.size());//This may break existing iterators so watch out for that
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
			frameBuffer.front().Transmit(true, ips, freq, udp);
			frameBuffer.pop_front();
		}

		std::cout << "Length: " << sData.length << " Sender: " << sData.sender.ip << " Data: " << complete << "\n";
	});
}

void timer(uvw::Loop &loop, std::map<std::string, int> &ips)
{
	auto timer = loop.resource<uvw::TimerHandle>();
	std::chrono::milliseconds duration(2000);
	timer->start(duration, duration);
	std::shared_ptr<uvw::UDPHandle> udp = loop.resource<uvw::UDPHandle>();

	timer->on<uvw::TimerEvent>([&ips, udp](const uvw::TimerEvent &, uvw::TimerHandle &timer) {
		std::chrono::milliseconds currentTime = pointToMilli(std::chrono::system_clock::now());
		std::chrono::milliseconds duration(1000);
		unsigned int numOfTransmits = 0;
		for (auto &i : frameBuffer)
		{
			std::chrono::milliseconds recievedTime = pointToMilli(i.recievedTime);
			if (currentTime > recievedTime)
			{
				numOfTransmits++;
			}
		}
		bool complete = true;
		for (int i = 0; i < numOfTransmits; i++)
		{
			for (auto &j : frameBuffer.front().sensorBuffer)
			{
				if (j.empty() || j.size() > j.front().value("size", 0))
				{
					complete = false;
				}
			}
			frameBuffer.front().Transmit(complete, ips, udp);
			frameBuffer.pop_front();
		}
	});
	
}

int main() {
	auto loop = uvw::Loop::getDefault();
	std::map<std::string, int> ips;
	listen(*loop, ips);
	timer(*loop, ips);
	loop->run();
	return 0;
}