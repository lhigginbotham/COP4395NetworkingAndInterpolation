#include <uvw.hpp>
#include <json.hpp>
#include <iostream>
#include <memory>


void listen(uvw::Loop &loop) {
	std::shared_ptr<uvw::UDPHandle> udp = loop.resource<uvw::UDPHandle>();
	udp->bind("127.0.0.1", 4951);
	udp->recv();

	udp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &err, uvw::UDPHandle &) {
		std::cout << "Code: " << err.code() << " Message: " << err.what() << "\n";
	});

	udp->on<uvw::UDPDataEvent>([](const uvw::UDPDataEvent &sData, uvw::UDPHandle &udp) {
		int test = 3;
		std::vector<std::string> ips = { "0" };
		if (!std::any_of(ips.begin(), ips.end(), sData.sender.ip))
		{
			ips.push_back(sData.sender.ip);
		}
		std::string result = sData.data.get();
		std::string complete = result.substr(0, sData.length);
		//So, this is bizarre.  Intellisense flags this as an error when passing a standard std::string but it compiles and runs regardless
		//Converting it to a Cstring causes Intellisense to no longer flag it
		//Unsure what a proper fix to this would be as library dev blames it on Intellisense (and the fact that it compiles and runs regardless supports that)

		nlohmann::json freq = nlohmann::json::parse(complete.c_str());
		//auto result2 = sData.data;
		std::cout << "Length: " << sData.length << " Sender: " << sData.sender.ip << " Data: " << complete << "\n";
	});
}

int main() {
	auto loop = uvw::Loop::getDefault();
	listen(*loop);
	loop->run();
}