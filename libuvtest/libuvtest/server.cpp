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
const ConfigStore globalConfig ("config.log");

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
			if(i.first == sData.sender.ip)
			{
				recieved = true;
				break;
			}
		}
		if (!recieved)
		{
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
		//std::chrono::milliseconds duration(1000);
		//std::chrono::milliseconds freqTime = time_tToMilli(freq.value("time", 0));
		//std::chrono::milliseconds lowerBound = pointToMilli(front->recievedTime);
		//std::chrono::milliseconds upperBound = lowerBound + duration;
		std::time_t duration = 1;
		std::time_t freqTime = freq.value("time", 0);
		std::time_t lowerBound = std::chrono::system_clock::to_time_t(front->recievedTime);
		std::time_t upperBound = lowerBound + duration;
		bool sendFuture = false;
		//Discard
		if (freqTime < lowerBound)
		{
			return;
		}
		//Place in current frame
		else if (freqTime >= lowerBound && (freqTime <= upperBound))
		{
			if (!front->sensorBuffer[ipPosition].empty())
			{
				for (int i=0; i<front->sensorBuffer[ipPosition].size(); i++)
				{
					if (front->sensorBuffer[ipPosition][i]["freqs"][0]["freq"] == freq["freqs"][0]["freq"])
					{
						sendFuture = true;
					}
				}
			}
			int size = front->sensorBuffer[ipPosition].size();
			//if (front->sensorBuffer[ipPosition].size() >= freq.value("size", 0))
			//{
			//	sendFuture = true;
			//}

			if (!sendFuture)
			{
				front->sensorBuffer[ipPosition].push_back(freq);
			}
		}
		//Place in future frame that can hold it
		else if(freqTime > upperBound || sendFuture)
		{
			bool inFrame = false, freqFound = false;
			for (auto &frame : frameBuffer)
			{
				if (freqTime < std::chrono::system_clock::to_time_t(frame.recievedTime))
				{
					inFrame = true;
					return;
				}
				if (freqTime >= std::chrono::system_clock::to_time_t(frame.recievedTime) && (freqTime < std::chrono::system_clock::to_time_t(frame.recievedTime) + duration))
				{
					for (int i = 0; i < frame.sensorBuffer[ipPosition].size(); i++)
					{
						if (frame.sensorBuffer[ipPosition][i]["freqs"][0]["freq"] == freq["freqs"][0]["freq"])
						{
							freqFound = true;
							break;
						}
					}
					//if (frame.sensorBuffer[ipPosition].size() >= freq.value("size", 0))
					//{
					//	freqFound = true;
					//}
					if (freqFound)
					{
						continue;
					}
					frame.sensorBuffer[ipPosition].push_back(freq);
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
				historicalBuffer.push_front(frameBuffer.front());
				frameBuffer.front().BatchSave(ips, 0, true);
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
	std::chrono::milliseconds duration;
	if (globalConfig.type == 0)
	{
		std::chrono::milliseconds duration(10000);
		timer->start(duration, duration);
	}
	else
	{
		std::chrono::milliseconds duration(5000);
		timer->start(duration, duration);
	}
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
		int posTracker = 0;
		for (int i = 0; i < numOfTransmits; i++)
		{
			for (auto &sensor : frameBuffer.front().sensorBuffer)
			{
				if (sensor.empty() || sensor.size() > sensor.front().value("size", 0))
				{
					if (sensor.empty())
					{
						std::string ip = ips[posTracker].first;
						misses.find(ip)->second++;
					}
					complete = false;
					break;
				}
				posTracker++;
			}
			if (globalConfig.type == 0)
			{
				frameBuffer.front().Transmit(complete, ips, udp);
			}
			if (globalConfig.type == 1)
			{
				historicalBuffer.push_front(frameBuffer.front());
				if (historicalBuffer.size() > 10)
				{
					historicalBuffer.pop_back();
				}
				frameBuffer.front().BatchSave(ips, 1, complete);
			}
			frameBuffer.pop_front();
		}
		if (numOfTransmits <= 0 && posTracker <= 0)
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
						if (ips.size() == (i+1))
						{
							ips[i].second--;
						}
					}
				}
			}
		}
		std::cout << "Exiting timer\n";
	});
	
}

void dbSave(uvw::Loop &loop, std::vector <std::pair<std::string, int>> &ips)
{
	auto timer = loop.resource<uvw::TimerHandle>();
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
				hFrame.BatchSave(ips, 1, completed);
				break;
			}
		}
		if (!historicalBuffer.empty())
		{
			historicalBuffer.front().BatchSave(ips, 1, completed);
		}
		historicalBuffer.clear();
	});
	//sql::mysql::MySQL_Driver *driver;
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
		//dbSave(*loop, ips);
	}
	loop->run();
	return 0;
}