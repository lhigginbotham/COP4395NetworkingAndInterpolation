#ifndef FRAME_HPP_
#define FRAME_HPP_

#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <vector>
#include <json.hpp>
#include <uvw.hpp>

#include "config.hpp"
#include "utilities.hpp"

class FrameBuffer
{
public:
	FrameBuffer(std::chrono::time_point<std::chrono::system_clock> time);
	~FrameBuffer() = default;
	bool LifetimeExceeded();
	void Transmit(bool complete, const std::map<std::string, int> &ips, const nlohmann::json &freq, uvw::UDPHandle &udp);
	void Transmit(bool complete, const std::map<std::string, int> &ips, std::shared_ptr<uvw::UDPHandle> udp);

	std::vector<std::vector<nlohmann::json>> sensorBuffer;
	std::chrono::time_point<std::chrono::system_clock> recievedTime;
private:
	std::chrono::duration<long long, std::milli> ElapsedTime(std::chrono::time_point<std::chrono::system_clock> startTime);

};

#endif