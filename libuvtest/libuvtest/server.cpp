#define _CRT_SECURE_NO_WARNINGS

#include <deque>
#include <iostream>
#include <json.hpp>
#include <uvw.hpp>

#include "config.hpp"
#include "frame.hpp"

#define MISS_THRESHOLD 6

//static std::vector<std::vector<std::vector<nlohmann::json>>> freqBuffer (10, std::vector<std::vector<nlohmann::json>>(0, std::vector<nlohmann::json> (0)));
static std::deque<FrameBuffer> frameBuffer;
static std::deque<FrameBuffer> historicalBuffer;
const ConfigStore globalConfig("config.log");
std::time_t lastSave = 0;

void listen(uvw::Loop &loop, std::vector <std::pair<std::string, int>> &ips, std::map<std::string, int> &misses) {
	std::shared_ptr<uvw::UDPHandle> udp = loop.resource<uvw::UDPHandle>();
	udp->bind("0.0.0.0", 4951);
	udp->recv();
	int cPosition = 0;

	udp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &err, uvw::UDPHandle &) {
		std::cout << "Code: " << err.code() << " Message: " << err.what() << "\n";
	});

	udp->on<uvw::UDPDataEvent>([&ips, &misses, cPosition](const uvw::UDPDataEvent &sData, uvw::UDPHandle &udp) {
		std::string result = sData.data.get();
		bool transmitBuffer = false;
		//Trim off excess data transmitted from client
		std::string complete = result.substr(0, sData.length);
		//So, this is bizarre.  Intellisense flags this as an error when passing a standard std::string but it compiles and runs regardless
		//Converting it to a Cstring causes Intellisense to no longer flag it
		//Unsure what a proper fix to this would be as library dev blames it on Intellisense (and the fact that it compiles and runs regardless supports that)
		nlohmann::json freq = nlohmann::json::parse(complete.c_str());

		//Check if the framebuffer data struct is empty
		if (frameBuffer.empty())
		{
			time_t recievedTime = freq.value("time", 0);
			frameBuffer.push_back(FrameBuffer(std::chrono::system_clock::from_time_t(recievedTime)));
		}

		//Check if incoming ip has been recieved before
		bool recieved = false;
		for (auto i : ips)
		{
			if (i.first == sData.sender.ip)
			{
				recieved = true;
				break;
			}
		}
		if (!recieved)
		{
			lastSave = freq.value("time", 0);
			ips.push_back(std::pair<std::string, int>(sData.sender.ip, ips.size()));
			misses.emplace(sData.sender.ip, 0);
			//Add a new sensorbuffer for current and stored future frames
			for (auto &frame : frameBuffer)
			{
				frame.sensorBuffer.push_back(std::vector<nlohmann::json>());
			}
			if (globalConfig.type == 0)
			{
				addSensorDatabase(freq);
			}

		}
		//If first use of sensor buffer, resize if to expected size
		if (frameBuffer[cPosition].sensorBuffer.size() != ips.size())
		{
			frameBuffer[cPosition].sensorBuffer.resize(ips.size());//This may break existing iterators so watch out for that
		}
		//Handle placements of incoming packets based on their time and the current frames time
		int ipPosition = -1;
		for (auto ip : ips)
		{
			if (ip.first == sData.sender.ip)
				ipPosition = ip.second;
		}
		auto front = frameBuffer.begin();
		std::chrono::milliseconds duration(500);
		std::chrono::milliseconds freqTime = time_tToMilli(freq.value("time", 0));
		std::chrono::milliseconds lowerBound = pointToMilli(front->recievedTime);
		std::chrono::milliseconds upperBound = lowerBound + duration;
		//Discard
		if (freqTime < lowerBound)
		{
			return;
		}
		//Place in current frame
		else if (freqTime >= lowerBound && (freqTime < upperBound))
		{
			front->sensorBuffer[ipPosition].push_back(freq);
		}
		//Place in future frame that can hold it
		else
		{
			bool inFrame = false;
			for (auto &i : frameBuffer)
			{
				if (freqTime >= pointToMilli(i.recievedTime) && (freqTime < pointToMilli(i.recievedTime) + duration))
				{
					i.sensorBuffer[ipPosition].push_back(freq);
					inFrame = true;
					break;
				}
			}
			if (!inFrame)
			{
				frameBuffer.push_back(FrameBuffer(std::chrono::system_clock::from_time_t(freq.value("time", 0))));
				//Using end() here causes a dereference error for some reason
				frameBuffer.back().sensorBuffer.push_back(std::vector<nlohmann::json>());
				frameBuffer.back().sensorBuffer.resize(ips.size());//This may break existing iterators so watch out for that
				frameBuffer.back().sensorBuffer[ipPosition].push_back(freq);
			}
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
			//Reset misses counter
			for (auto &miss : misses)
			{
				miss.second = 0;
			}
			if (globalConfig.type == 0)
			{
				frameBuffer.front().Transmit(true, ips, freq, udp);
				frameBuffer.pop_front();
			}
			else if (globalConfig.type == 1)
			{
				std::cout << "Allocating frame buffer\n";
				historicalBuffer.push_front(frameBuffer.front());
				frameBuffer.front().BatchSave(ips, 0);
				if (historicalBuffer.size() > 10)
				{
					historicalBuffer.pop_back();
				}
				frameBuffer.pop_front();
			}
		}

		//std::cout << "Length: " << sData.length << " Sender: " << sData.sender.ip << " Data: " << complete << "\n";
	});
}

void timer(uvw::Loop &loop, std::vector <std::pair<std::string, int>> &ips, std::map<std::string, int> &misses)
{
	auto timer = loop.resource<uvw::TimerHandle>();
	std::chrono::milliseconds duration(2000);
	timer->start(duration, duration);
	std::shared_ptr<uvw::UDPHandle> udp = loop.resource<uvw::UDPHandle>();

	timer->on<uvw::TimerEvent>([&ips, &misses, udp](const uvw::TimerEvent &, uvw::TimerHandle &timer) {
		std::cout << "Entering Timer\n";
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
		int posTracker = 0, activeTracker = 0;
		for (int i = 0; i < numOfTransmits; i++)
		{
			for (auto &j : frameBuffer.front().sensorBuffer)
			{
				if (j.empty() || j.size() < j.front().value("size", 0))
				{
					if (j.empty())
					{
						std::string ip = ips[posTracker].first;
						misses.find(ip)->second++;
					}
					complete = false;
					break;
				}
				posTracker++;
				activeTracker++;
			}
			if (globalConfig.type == 0)
			{
				frameBuffer.front().Transmit(complete, ips, udp);
			}
			else
			{
				historicalBuffer.push_front(frameBuffer.front());
				if (historicalBuffer.size() > 10)
				{
					historicalBuffer.pop_back();
				}
			}
			frameBuffer.pop_front();
			posTracker = 0;
		}
		if (numOfTransmits <= 0 && activeTracker <= 0)
		{
			//Add misses to all sensors if frame is empty
			if (frameBuffer.empty())
			{
				for (auto &miss : misses)
				{
					miss.second++;
				}
			}
		}
		std::string baseip = "";
		if (!misses.empty())
		{
			for (auto &miss : misses)
			{
				if (miss.second >= MISS_THRESHOLD)
				{
					baseip = miss.first;
				}
			}
			if (baseip != "")
			{
				misses.erase(baseip);
				bool alterPosition = false;
				for (int i = 0; i < ips.size(); i++)
				{
					if (alterPosition == true)
					{
						ips[i].second--;
					}
					if (ips[i].first == baseip)
					{
						ips.erase(ips.begin() + i);
						alterPosition = true;
						//Account for removing second to last element
						if (ips.size() == (i + 1))
						{
							ips[i].second--;
						}
					}
				}
			}
		}
	});

}

void dbSave(uvw::Loop &loop, std::vector <std::pair<std::string, int>> &ips)
{
/*	auto timer = loop.resource<uvw::TimerHandle>();
	std::chrono::seconds duration(10);
	std::shared_ptr<uvw::UDPHandle> udp = loop.resource<uvw::UDPHandle>();
	timer->start(duration, duration);

	timer->on<uvw::TimerEvent>([&ips, udp](const uvw::TimerEvent &, uvw::TimerHandle &timer) {
		bool completed = true;
		for (auto hFrame : historicalBuffer)
		{
			for (int i = 0; i < ips.size(); i++)
			{
				for (int j = 0; j < hFrame.sensorBuffer[i].size(); j++)
				{
					for (int k = 0; k < hFrame.sensorBuffer[i][j]["freqs"].size(); k++)
					{
						bool comp = hFrame.sensorBuffer[i][j]["complete"];
						if (!comp)
						{
							completed = false;
						}
					}
				}
			}
			if (!completed)
			{
				std::cout << "Saving first complete buffer\n";
				hFrame.BatchSave(ips, 1);
				break;
			}
			std::cout << "Completed is: " << completed << "\n";
			if (!historicalBuffer.empty())
			{
				historicalBuffer.front().BatchSave(ips, 1);
			}
			int size = historicalBuffer.size();
			for (int i = 0; i < size; i++)
			{
				historicalBuffer.pop_front();
			}
			//historicalBuffer.clear();
			std::cout << "Historical Buffer: " << historicalBuffer.size() << "\n";
		}
		std::cout << "Exiting DBSave\n";
	});*/
}

int main() {
	auto loop = uvw::Loop::getDefault();
	std::vector <std::pair<std::string, int>> ips;
	//std::map<std::string, int> ips;
	std::map<std::string, int> misses;
	listen(*loop, ips, misses);
	timer(*loop, ips, misses);
	if (globalConfig.type == 1)
	{
		dbSave(*loop, ips);
	}
	loop->run();
	return 0;
}
