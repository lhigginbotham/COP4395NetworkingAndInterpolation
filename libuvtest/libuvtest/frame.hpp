#ifndef FRAME_HPP_
#define FRAME_HPP_

#include <chrono>
#include <vector>
#include <json.hpp>

#include "utilities.hpp"

class FrameBuffer
{
public:
	FrameBuffer(std::chrono::time_point<std::chrono::system_clock> time);
	~FrameBuffer() = default;
	bool LifetimeExceeded();

	std::vector<std::vector<nlohmann::json>> sensorBuffer;
	std::chrono::time_point<std::chrono::system_clock> recievedTime;
private:
	std::chrono::duration<long long, std::milli> ElapsedTime(std::chrono::time_point<std::chrono::system_clock> startTime);

};

#endif
