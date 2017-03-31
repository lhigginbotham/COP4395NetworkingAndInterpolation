#define _CRT_SECURE_NO_WARNINGS

#include <uvw.hpp>
#include <json.hpp>
#include <iostream>
#include <memory>

#include "config.hpp"

static std::map<std::string, int> ips;
static std::vector<std::vector<std::vector<nlohmann::json>>> freqBuffer (10, std::vector<std::vector<nlohmann::json>>(0, std::vector<nlohmann::json> (0)));
static const ConfigStore globalConfig ("config.log");

void listen(uvw::Loop &loop) {
	std::shared_ptr<uvw::UDPHandle> udp = loop.resource<uvw::UDPHandle>();
	udp->bind("127.0.0.1", 4951);
	udp->recv();

	udp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &err, uvw::UDPHandle &) {
		std::cout << "Code: " << err.code() << " Message: " << err.what() << "\n";
	});

	udp->on<uvw::UDPDataEvent>([](const uvw::UDPDataEvent &sData, uvw::UDPHandle &udp) {
		std::string result = sData.data.get();
		bool transmitBuffer = false;
		//Trim off excess data transmitted from client
		std::string complete = result.substr(0, sData.length);
		//So, this is bizarre.  Intellisense flags this as an error when passing a standard std::string but it compiles and runs regardless
		//Converting it to a Cstring causes Intellisense to no longer flag it
		//Unsure what a proper fix to this would be as library dev blames it on Intellisense (and the fact that it compiles and runs regardless supports that)
		nlohmann::json freq = nlohmann::json::parse(complete.c_str());
		int num = freq.value("number", 0);

		if (ips.find(sData.sender.ip) == ips.end())
		{
			ips.emplace(sData.sender.ip, ips.size());
			freqBuffer[num].push_back(std::vector<nlohmann::json>());
		}

		int ipPosition = ips.find(sData.sender.ip)->second;
		freqBuffer[num][ipPosition].push_back(freq);

		int vFullTracker = 0;
		if (freqBuffer[num][ipPosition].size() >= freq.value("size", 0))
		{
			for (auto &&i : freqBuffer[num])
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
					std::string message = freqBuffer[num][i][j].dump();
					uvw::Addr addr;
					std::string ip = globalConfig.config.value("sendip", "-1");
					unsigned int port = 4951;
					char* data = new char[message.length() + 1];
					std::strcpy(data, message.c_str());
					unsigned int len = message.length();
					udp.send(ip, port, data, len);
					delete[] data;
				}
				freqBuffer[num][i].resize(0);
				freqBuffer[num][i].shrink_to_fit();
			}
			
		}

		std::cout << "Length: " << sData.length << " Sender: " << sData.sender.ip << " Data: " << complete << "\n";
	});
}

int main() {
	auto loop = uvw::Loop::getDefault();
	listen(*loop);
	loop->run();
	return 0;
}