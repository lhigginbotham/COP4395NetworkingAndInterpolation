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
	auto endTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	return pointToMilli(endTime) - pointToMilli(startTime);
}