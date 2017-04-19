
#include "utilities.hpp"

std::chrono::duration<long long, std::milli> pointToMilli(std::chrono::time_point<std::chrono::system_clock> point)
{
	auto nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(point);
	std::chrono::milliseconds epoch = nowMs.time_since_epoch();
	auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
	int duration = value.count();
	std::chrono::duration<long long, std::milli> mTime(duration);
	return mTime;
}

std::chrono::duration<long long, std::milli> time_tToMilli(std::time_t time)
{
	std::chrono::system_clock::time_point nPoint = std::chrono::system_clock::from_time_t(time);
	return pointToMilli(nPoint);
}

bool addSensorDatabase(const nlohmann::json &message)
{
	try {
		sql::Driver *driver;
		sql::Connection *conn;
		sql::ResultSet *res;
		std::string ip = globalConfig.config.value("databaseIP", "");
		std::string port = std::to_string(globalConfig.config.value("databasePort", -1));
		std::string connectionStr = "tcp://" + ip + ":" + port;
		
		driver = get_driver_instance();
		conn = driver->connect(connectionStr.c_str(), globalConfig.config.value("databaseUsername", "").c_str(),
			globalConfig.config.value("databasePassword", "").c_str());
		conn->setSchema("mydb");

		sql::PreparedStatement *prep_stmt;
		prep_stmt = conn->prepareStatement("SELECT * FROM sensors WHERE SID=?");
		nlohmann::json freq = message["freqs"][0];
		int sid = freq.value("id", -1);
		double lat = message["lat"];
		double longCoord = message["long"];
		prep_stmt->setInt(1, sid);

		res = prep_stmt->executeQuery();
		if (!res->next())
		{
			std::cout << "Is null\n";
			delete prep_stmt;
			prep_stmt = conn->prepareStatement("INSERT INTO sensors(SID, Latitude, Longitude) VALUES (?, ?, ?)");
			prep_stmt->setInt(1, sid);
			prep_stmt->setDouble(2, lat);
			prep_stmt->setDouble(3, longCoord);
			prep_stmt->executeQuery();
		}
		delete res;
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