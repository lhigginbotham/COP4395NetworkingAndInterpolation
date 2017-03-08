#include <iostream>;
#include <vector>;
#include <math.h>
using namespace std;

// copy over to main file later
struct MeasuredPoint
{
	double x;
	double y;
	double measuredStrength;
};

struct Point
{
	double x;
	double y;
};


double getDistance(double x1,double y1,double x2, double y2)
{
	double distance;
	double xdistance, ydistance;
	xdistance = pow((x1 - x2), 2);
	ydistance = pow((y1 - y2), 2);
	distance = sqrt(xdistance + ydistance);
	



	return distance;
}

std::vector<MeasuredPoint> interpolation(std::vector <MeasuredPoint> sensors, std::vector <Point> pointsToEstimate)
{
	
	double sumOfTop=0;
	double sumOfBottom=0;
	MeasuredPoint temp;
	unsigned int i,j;
	for (i = 0; i < pointsToEstimate.size(); i++)
	{
		for (j = 0; j < sensors.size(); j++)
		{
			sumOfTop += (pow(getDistance(sensors.at(j).x, sensors.at(j).y,pointsToEstimate.at(i).x, pointsToEstimate.at(i).y),-2)*sensors.at(j).measuredStrength);
			sumOfBottom += pow(getDistance(sensors.at(j).x, sensors.at(j).y, pointsToEstimate.at(i).x, pointsToEstimate.at(i).y), -2);
			
		}
		temp.x = pointsToEstimate.at(i).x;
		temp.y = pointsToEstimate.at(i).y;
		temp.measuredStrength = sumOfTop / sumOfBottom;
		sensors.push_back(temp);

		sumOfTop = 0;
		sumOfBottom = 0;

	}

	return sensors;
}



int main()
{
	std::vector <MeasuredPoint> sensors = { {0,0,60},{10,0,30},{10,10,25},{0,10,32}};
	std::vector <Point> pointsToEstimate = { {5,5},{5,8},{5,3}, {3,5},{8,5}};

	sensors = interpolation(sensors, pointsToEstimate);
	unsigned int i;
	for (i = 0; i < sensors.size(); i++)
	{
		cout << "\n(" << sensors.at(i).x <<"," << sensors.at(i).y << ")   has a power of " << sensors.at(i).measuredStrength;
	}
	
	



	int x;
	cin >> x;



	return 0;
}
