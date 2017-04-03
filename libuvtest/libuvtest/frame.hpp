#ifndef FRAME_HPP_
#define FRAME_HPP_

#include <vector>
#include <json.hpp>
#include <chrono>

class FrameBuffer
{
public:
	FrameBuffer();
	~FrameBuffer() = default;
	bool LifetimeExceeded();

	std::vector<std::vector<nlohmann::json>> freqBuffer;
	std::chrono::time_point<std::chrono::system_clock> recievedTime;
private:
	std::chrono::duration<long long, std::milli> ElapsedTime(std::chrono::time_point<std::chrono::system_clock> startTime);

};

#endif
