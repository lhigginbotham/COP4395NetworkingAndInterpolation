/*
	Open scource code references

	Point in polygon section Reference:
	Title: How to check if a given point lies inside or outside a polygon?
	Author: Unknown
	URL: http://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/


	2D convex hull Reference:
	Title: Algorithm Implementation/Geometry/Convex hull/Monotone chain
	Author: Unknown
	URL: https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain


*/





#include <iostream>
#include <stack>
#include <stdlib.h>
#include <vector>
#include <algorithm>




using namespace std;

typedef double coord_t;         // coordinate type
typedef double coord2_t;  // must be big enough to hold 2*max(|coordinate|)^2

struct MeasuredPoint {
	coord_t x, y, measuredStrength;

	bool operator <(const MeasuredPoint &p) const {
		return x < p.x || (x == p.x && y < p.y);
	}
};

// 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
// Returns a positive value, if OAB makes a counter-clockwise turn,
// negative for clockwise turn, and zero if the points are collinear.
coord2_t cross(const MeasuredPoint &O, const MeasuredPoint &A, const MeasuredPoint &B)
{
	return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// Returns a list of points on the convex hull 
// Note: the last point in the returned list is the same as the first one.
vector<MeasuredPoint> convex_hull(std::vector<MeasuredPoint> P)
{
	int n = P.size(), k = 0;
	if (n == 1) return P;
	vector<MeasuredPoint> H(2 * n);

	// Sort points 
	sort(P.begin(), P.end());

	// Build lower hull
	for (int i = 0; i < n; ++i) {
		while (k >= 2 && cross(H[k - 2], H[k - 1], P[i]) <= 0) k--;
		H[k++] = P[i];
	}

	// Build upper hull
	for (int i = n - 2, t = k + 1; i >= 0; i--) {
		while (k >= t && cross(H[k - 2], H[k - 1], P[i]) <= 0) k--;
		H[k++] = P[i];
	}

	H.resize(k - 1);
	return H;
}


// Function to obtain square of the distance between p1 and p2
double distSq(MeasuredPoint p1, MeasuredPoint p2)
{
	return (p1.x - p2.x)*(p1.x - p2.x) +
		(p1.y - p2.y)*(p1.y - p2.y);
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(MeasuredPoint p, MeasuredPoint q, MeasuredPoint r)
{
	double val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	if (val == 0) return 0;  // colinear
	return (val > 0) ? 1 : 2; // clock or counterclock wise
}





// Define Infinite (Using INT_MAX caused overflow problems)
#define INF 10000000



// Given three colinear MeasuredPoints p, q, r, the function checks if
// MeasuredPoint q lies on line segment 'pr'
bool onSegment(MeasuredPoint p, MeasuredPoint q, MeasuredPoint r)
{
	if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
		q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
		return true;
	return false;
}



// The function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool doIntersect(MeasuredPoint p1, MeasuredPoint q1, MeasuredPoint p2, MeasuredPoint q2)
{
	// Find the four orientations needed for general and
	// special cases
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	// General case
	if (o1 != o2 && o3 != o4)
		return true;

	// Special Cases
	// p1, q1 and p2 are colinear and p2 lies on segment p1q1
	if (o1 == 0 && onSegment(p1, p2, q1)) return true;

	// p1, q1 and p2 are colinear and q2 lies on segment p1q1
	if (o2 == 0 && onSegment(p1, q2, q1)) return true;

	// p2, q2 and p1 are colinear and p1 lies on segment p2q2
	if (o3 == 0 && onSegment(p2, p1, q2)) return true;

	// p2, q2 and q1 are colinear and q1 lies on segment p2q2
	if (o4 == 0 && onSegment(p2, q1, q2)) return true;

	return false; // Doesn't fall in any of the above cases
}

// Returns true if the MeasuredPoint p lies inside the polygon[] with n vertices
bool isInside(std::vector<MeasuredPoint> polygon, int n, MeasuredPoint p)
{
	// There must be at least 3 vertices in polygon[]
	if (n < 3)  return false;

	// Create a MeasuredPoint for line segment from p to infinite
	MeasuredPoint extreme = { INF, p.y };

	// Count intersections of the above line with sides of polygon
	int count = 0, i = 0;
	do
	{
		int next = (i + 1) % n;

		// Check if the line segment from 'p' to 'extreme' intersects
		// with the line segment from 'polygon[i]' to 'polygon[next]'
		if (doIntersect(polygon[i], polygon[next], p, extreme))
		{
			// If the MeasuredPoint 'p' is colinear with line segment 'i-next',
			// then check if it lies on segment. If it lies, return true,
			// otherwise false
			if (orientation(polygon[i], p, polygon[next]) == 0)
				return onSegment(polygon[i], p, polygon[next]);

			count++;
		}
		i = next;
	} while (i != 0);

	// Return true if count is odd, false otherwise
	return count & 1;  // Same as (count%2 == 1)
}





MeasuredPoint getCenter(std::vector<MeasuredPoint> MeasuredPoints)
{
	MeasuredPoint center;
	double xtotal = 0;
	double ytotal = 0;
	double masstotal = 0;
	int i;
	for (i = 0; i<MeasuredPoints.size(); i++)
	{
		xtotal += MeasuredPoints[i].x;
		ytotal += MeasuredPoints[i].y;

	}
	masstotal = MeasuredPoints.size();
	//cout << masstotal <<"\n";
	center.x = xtotal / masstotal;
	center.y = ytotal / masstotal;

	//cout << masstotal;
	return center;

}


std::vector <MeasuredPoint>  floodFill(MeasuredPoint current, double interval, std::vector<MeasuredPoint> sensorLoc, std::vector <MeasuredPoint> estimatedPoints)
{
	//cout << "checking (" << current.x << "," << current.y << ") ... ";
	unsigned int i;
	bool visited = false;

	for (i = 0; i<estimatedPoints.size(); i++)
	{
		if (estimatedPoints.at(i).x == current.x && estimatedPoints.at(i).y == current.y)
		{
			visited = true;
			break;
		}
	}
	if ((isInside(sensorLoc, sensorLoc.size(), current)) == true && visited == false)
	{
		//cout << "Suceeded!\n";
		estimatedPoints.push_back(current);
	}
	else
	{
		//cout << "Failed!\n";
		return estimatedPoints;
	}
	//up
	MeasuredPoint p;
	p = current;
	p.y = p.y + interval;
	estimatedPoints = floodFill(p, interval, sensorLoc, estimatedPoints);
	//right
	p = current;
	p.x = p.x + interval;
	estimatedPoints = floodFill(p, interval, sensorLoc, estimatedPoints);

	//left
	p = current;
	p.x = p.x - interval;
	estimatedPoints = floodFill(p, interval, sensorLoc, estimatedPoints);
	//down
	p = current;
	p.y = p.y - interval;
	estimatedPoints = floodFill(p, interval, sensorLoc, estimatedPoints);
	return estimatedPoints;
}


double getDistance(double x1, double y1, double x2, double y2)
{
	double distance;
	double xdistance, ydistance;
	xdistance = pow((x1 - x2), 2);
	ydistance = pow((y1 - y2), 2);
	distance = sqrt(xdistance + ydistance);

	return distance;
}

std::vector<MeasuredPoint> interpolation(std::vector <MeasuredPoint> sensors, std::vector <MeasuredPoint> pointsToEstimate)
{

	double sumOfTop = 0;
	double sumOfBottom = 0;
	std::vector<MeasuredPoint> newPoints;
	MeasuredPoint temp;
	int isAlreadyDone = 0;
	unsigned int i, j;
	for (i = 0; i < pointsToEstimate.size(); i++)
	{
		for (j = 0; j < sensors.size(); j++)
		{
			sumOfTop += (pow(getDistance(sensors.at(j).x, sensors.at(j).y, pointsToEstimate.at(i).x, pointsToEstimate.at(i).y), -2)*sensors.at(j).measuredStrength);
			sumOfBottom += pow(getDistance(sensors.at(j).x, sensors.at(j).y, pointsToEstimate.at(i).x, pointsToEstimate.at(i).y), -2);

		}
		temp.x = pointsToEstimate.at(i).x;
		temp.y = pointsToEstimate.at(i).y;
		temp.measuredStrength = sumOfTop / sumOfBottom;


		for (j = 0; j < sensors.size(); j++)
		{
			if (temp.y == sensors.at(j).y && temp.y == sensors.at(j).y)
			{
				isAlreadyDone = 1;

			}


		}

		if (isAlreadyDone == 0)
		{
			newPoints.push_back(temp);
		}
		isAlreadyDone = 0;

		sumOfTop = 0;
		sumOfBottom = 0;

	}

	return newPoints;
}






//name get MeasuredPoints later
int main()
{
	unsigned int i;
	int x;

	//std::vector<MeasuredPoint> sensors = { { 28.600367,-81.198041, 60.0 } ,{ 28.600693, -81.198229, 35.0 } ,{ 28.600893, -81.197376, 45.0 } ,{ 28.600700, -81.197134, 22.0 } };
	std::vector<MeasuredPoint> sensors = { { 0, 0, 60.0 } ,{ 0,10, 35.0 } ,{ 10, 10, 45.0 } ,{ 10,0, 22.0 },{ 5, 5, 22 } };
	std::vector <MeasuredPoint> estimatedPoints;
	std::vector<MeasuredPoint> polygon = convex_hull(sensors);
	for (i = 0; i<polygon.size(); i++)
	{

		cout << "(" << std::fixed << polygon.at(i).x << "," << polygon.at(i).y << ") at a power of " << polygon.at(i).measuredStrength << "\n";
	}

	cin >> x;


	MeasuredPoint center = getCenter(polygon);
	//cout << "(" << std::fixed << center.x << "," << center.y << ")\n\n";

	double interval = 2.0005;

	estimatedPoints = floodFill(center, interval, sensors, estimatedPoints);

	std::vector<MeasuredPoint> list = interpolation(sensors, estimatedPoints);


	for (i = 0; i<list.size(); i++)
	{

		cout << "(" << std::fixed << list.at(i).x << "," << list.at(i).y << ") at a power of " << list.at(i).measuredStrength << "\n";
	}


	cin >> x;







	return 0;
}




