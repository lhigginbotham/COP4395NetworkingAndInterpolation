#include "frame.hpp"

FrameBuffer::FrameBuffer(std::chrono::time_point<std::chrono::system_clock> time) : recievedTime(time)
{

}


bool FrameBuffer::LifetimeExceeded() 
{
	std::chrono::duration<long long, std::milli> duration(500);
	return ElapsedTime(this->recievedTime) >= duration ? true : false;
}

std::chrono::duration<long long, std::milli> FrameBuffer::ElapsedTime(std::chrono::time_point<std::chrono::system_clock> startTime)
{
	auto nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	std::chrono::milliseconds epoch = nowMs.time_since_epoch();
	auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
	long duration = value.count();
	std::chrono::duration<long long, std::milli> endTime(duration);

	nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(startTime);
	epoch = nowMs.time_since_epoch();
	value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
	duration = value.count();
	std::chrono::duration<long long, std::milli> sTime(duration);
	return endTime - sTime;
}