#include "frame.hpp"

FrameBuffer::FrameBuffer(std::chrono::time_point<std::chrono::system_clock> time) : recievedTime(time)
{

}


bool FrameBuffer::LifetimeExceeded()
{
std::chrono::duration<long long, std::milli> duration(500);
return ElapsedTime(this->recievedTime) >= duration ? true : false;
}

void FrameBuffer::Transmit(bool complete, const std::vector <std::pair<std::string, int>> &ips, const nlohmann::json &message, uvw::UDPHandle &udp)
{
	std::chrono::time_point<std::chrono::system_clock> systemTime = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(systemTime);
	int totalSize = ips.size() * message.value("size", 0);
	std::queue<nlohmann::json> frequencies;
	for (int i = 0; i < ips.size(); i++)
	{
		for (int j = 0; j < this->sensorBuffer[i].size(); j++)
		{
			for (int k = 0; k < this->sensorBuffer[i][j]["freqs"].size(); k++)
			{
				frequencies.push(this->sensorBuffer[i][j]["freqs"][k]);
			}
		}
		this->sensorBuffer[i].resize(0);
		this->sensorBuffer[i].shrink_to_fit();
	}
	std::vector<nlohmann::json> freq;

	//TODO: Implement filtering here

	while (!frequencies.empty())
	{
		for (int i = 0; i < 5; i++)
		{
			if (frequencies.empty())
				break;
			freq.push_back(frequencies.front());
			frequencies.pop();
		}
		nlohmann::json alteredMessage;
		alteredMessage["complete"] = true;
		alteredMessage["freqs"] = freq;
		alteredMessage["size"] = totalSize;
		alteredMessage["time"] = currentTime;
		std::string message = alteredMessage.dump();
		std::string ip = globalConfig.config.value("sendip", "-1");
		unsigned int port = 4951;
		//send won't take message.c_str()
		auto testData = std::make_unique<char[]>(512);
		for (int i = 0; i < message.length(); i++)
		{
			testData[i] = message[i];
		}
		unsigned int len = message.length();
		udp.send(ip, port, std::move(testData), len);
		freq.resize(0);
	}
}

void FrameBuffer::Transmit(bool complete, const std::vector <std::pair<std::string, int>> &ips, std::shared_ptr<uvw::UDPHandle> udp)
{
	std::chrono::time_point<std::chrono::system_clock> systemTime = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(systemTime);

	std::queue<nlohmann::json> frequencies;
	for (int i = 0; i < ips.size(); i++)
	{
		for (int j = 0; j < this->sensorBuffer[i].size(); j++)
		{
			for (int k = 0; k < this->sensorBuffer[i][j]["freqs"].size(); k++)
			{
				frequencies.push(this->sensorBuffer[i][j]["freqs"][k]);
			}
		}
		this->sensorBuffer[i].resize(0);
		this->sensorBuffer[i].shrink_to_fit();
	}

	//TODO Implement filtering

	std::vector<nlohmann::json> freq;
	int totalSize = (frequencies.size() + (5 - 1)) / 5;

	while (!frequencies.empty())
	{
		for (int i = 0; i < 5; i++)
		{
			if (frequencies.empty())
				break;
			freq.push_back(frequencies.front());
			frequencies.pop();
		}
		nlohmann::json alteredMessage;
		alteredMessage["complete"] = complete;
		alteredMessage["freqs"] = freq;
		alteredMessage["size"] = totalSize;
		alteredMessage["time"] = currentTime;
		std::string message = alteredMessage.dump();
		std::string ip = globalConfig.config.value("sendip", "-1");
		unsigned int port = 4951;
		//send won't take message.c_str()
		auto testData = std::make_unique<char[]>(512);
		for (int i = 0; i < message.length(); i++)
		{
			testData[i] = message[i];
		}

		unsigned int len = message.length();
		udp->send(ip, port, std::move(testData), len);
		freq.resize(0);
	}
}

bool FrameBuffer::BatchSave(const std::vector <std::pair<std::string, int>> &ips)
{
	std::vector<nlohmann::json> frequencies;
	bool completed = true;
	for (int i = 0; i < ips.size(); i++)
	{
		for (int j = 0; j < this->sensorBuffer[i].size(); j++)
		{
			for (int k = 0; k < this->sensorBuffer[i][j]["freqs"].size(); k++)
			{
				frequencies.push_back(this->sensorBuffer[i][j]["freqs"][k]);
				bool comp = this->sensorBuffer[i][j]["complete"];
				if (!comp)
				{
					completed = false;
				}
			}
		}
		this->sensorBuffer[i].resize(0);
		this->sensorBuffer[i].shrink_to_fit();
	}
	std::string vals = "";
	for (int i = 0; i < frequencies.size(); i++)
	{
		if (i == frequencies.size() - 1)
			vals += "(?, ?, ?, ?, ?)";
		else
			vals += "(?, ?, ?, ?, ?), ";
	}
	std::string ins = "INSERT INTO live(TIME, Completed, Frequency, Readings, Sensors_SID) VALUES " + vals;
	sql::SQLString sqlIns = ins.c_str();
	try {
		sql::Driver *driver;
		sql::Connection *conn;
		std::string ip = globalConfig.config.value("databaseIP", "");
		std::string port = std::to_string(globalConfig.config.value("databasePort", -1));
		std::string connectionStr = "tcp://" + ip + ":" + port;
		
		driver = get_driver_instance();
		conn = driver->connect(connectionStr.c_str(), globalConfig.config.value("databaseUsername", "").c_str(),
			globalConfig.config.value("databasePassword", "").c_str());
		conn->setSchema("mydb");

		sql::PreparedStatement *prep_stmt;
		prep_stmt = conn->prepareStatement(sqlIns);
		//Start at one here to match up expected indexing with prepared statements
		std::time_t time = 0;
		int stmtPos = 1;
		for (int i = 0; i < frequencies.size(); i++)
		{
			time = frequencies[i].value("time", 0);
			auto ltm = localtime(&time);
			std::string sTime = std::to_string(1900 + ltm->tm_year) + "-" + std::to_string(ltm->tm_mon) + "-" + std::to_string(ltm->tm_mday)
					+ " " + std::to_string(ltm->tm_hour) + ":" + std::to_string(ltm->tm_min) + ":" + std::to_string(ltm->tm_sec);
			sql::SQLString sqlTime = sTime.c_str();
			prep_stmt->setDateTime(stmtPos++, sqlTime);
			prep_stmt->setBoolean(stmtPos++, completed);
			prep_stmt->setInt(stmtPos++, frequencies[i]["frequency"]);
			prep_stmt->setInt(stmtPos++, frequencies[i]["reading"]);
			prep_stmt->setInt(stmtPos++, frequencies[i]["sensor-id"]);
		}
		prep_stmt->execute();
		delete prep_stmt;
		delete conn;
		return true;
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << "\n";
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )\n";
	}
	return false;
}

std::chrono::duration<long long, std::milli> FrameBuffer::ElapsedTime(std::chrono::time_point<std::chrono::system_clock> startTime)
{
	auto endTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	return pointToMilli(endTime) - pointToMilli(startTime);
}