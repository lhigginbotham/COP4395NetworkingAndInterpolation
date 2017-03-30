#include <iostream>
#include <stack>
#include <stdlib.h>
#include <vector>
#include <algorithm>

using namespace std;


struct MeasuredPoint
{
	double x, y, measuredStrength;

};



//needed for  sorting MeasuredPoints with referenceto  the first MeasuredPoint 
MeasuredPoint p0;

// A utility function to find next to top in a stack
MeasuredPoint nextToTop(stack<MeasuredPoint> &S)
{
	MeasuredPoint p = S.top();
	S.pop();
	MeasuredPoint res = S.top();
	S.push(p);
	return res;
}

// A utility function to swap two MeasuredPoints
void swap(MeasuredPoint &p1, MeasuredPoint &p2)
{
	MeasuredPoint temp = p1;
	p1 = p2;
	p2 = temp;
}

// A utility function to return square of distance
// between p1 and p2
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

// A function used by library function qsort() to sort an array of
// MeasuredPoints with respect to the first MeasuredPoint
int compare(const void *vp1, const void *vp2)
{
	MeasuredPoint *p1 = (MeasuredPoint *)vp1;
	MeasuredPoint *p2 = (MeasuredPoint *)vp2;

	// Find orientation
	int o = orientation(p0, *p1, *p2);
	if (o == 0)
		return (distSq(p0, *p2) >= distSq(p0, *p1)) ? -1 : 1;

	return (o == 2) ? -1 : 1;
}

// Prints convex hull of a set of n MeasuredPoints.
std::vector<MeasuredPoint> convexHull(std::vector<MeasuredPoint> MeasuredPoints, int n)
{
	std::vector <MeasuredPoint> output;
	// Find the bottommost MeasuredPoint
	double ymin = MeasuredPoints[0].y, min = 0;
	for (int i = 1; i < n; i++)
	{
		double y = MeasuredPoints[i].y;

		// Pick the bottom-most or chose the left
		// most MeasuredPoint in case of tie
		if ((y < ymin) || (ymin == y &&
			MeasuredPoints[i].x < MeasuredPoints[min].x))
			ymin = MeasuredPoints[i].y, min = i;
	}

	// Place the bottom-most MeasuredPoint at first position
	swap(MeasuredPoints[0], MeasuredPoints[min]);

	// Sort n-1 MeasuredPoints with respect to the first MeasuredPoint.
	// A MeasuredPoint p1 comes before p2 in sorted ouput if p2
	// has larger polar angle (in counterclockwise
	// direction) than p1
	p0 = MeasuredPoints[0];
	qsort(&MeasuredPoints[1], n - 1, sizeof(MeasuredPoint), compare);

	// If two or more MeasuredPoints make same angle with p0,
	// Remove all but the one that is farthest from p0
	// Remember that, in above sorting, our criteria was
	// to keep the farthest MeasuredPoint at the end when more than
	// one MeasuredPoints have same angle.
	int m = 1; // Initialize size of modified array
	for (int i = 1; i<n; i++)
	{
		// Keep removing i while angle of i and i+1 is same
		// with respect to p0
		while (i < n - 1 && orientation(p0, MeasuredPoints[i],
			MeasuredPoints[i + 1]) == 0)
			i++;


		MeasuredPoints[m] = MeasuredPoints[i];
		m++;  // Update size of modified array
	}




	// Create an empty stack and push first three MeasuredPoints
	// to it.
	stack<MeasuredPoint> S;
	S.push(MeasuredPoints[0]);
	S.push(MeasuredPoints[1]);
	S.push(MeasuredPoints[2]);

	// Process remaining n-3 MeasuredPoints
	for (int i = 3; i < m; i++)
	{
		// Keep removing top while the angle formed by
		// MeasuredPoints next-to-top, top, and MeasuredPoints[i] makes
		// a non-left turn
		while (orientation(nextToTop(S), S.top(), MeasuredPoints[i]) != 2)
			S.pop();
		S.push(MeasuredPoints[i]);
	}
	int a = 0;
	// Now stack has the output MeasuredPoints, print contents of stack
	while (!S.empty())
	{
		MeasuredPoint p = S.top();

		output.push_back(p);

		S.pop();
	}
	return output;
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







//name get MeasuredPoints later
int main()
{
	unsigned int i;
	int x;

	std::vector<MeasuredPoint> sensors = { { 28.600367,-81.198041, 60.0 } ,{ 28.600693, -81.198229, 35.0 } ,{ 28.600893, -81.197376, 45.0 } ,{ 28.600700, -81.197134, 22.0 } };

	std::vector <MeasuredPoint> estimatedPoints;
	std::vector<MeasuredPoint> polygon = convexHull(sensors, sensors.size());
	MeasuredPoint center = getCenter(polygon);
	cout << "(" << std::fixed << center.x << "," << center.y << ")\n\n";
	double interval = .0005;
	estimatedPoints = floodFill(center, interval, sensors, estimatedPoints);

	for (i = 0; i<estimatedPoints.size(); i++)
	{

		cout << "(" << std::fixed << estimatedPoints.at(i).x << "," << estimatedPoints.at(i).y << ")\n";
	}

	cin >> x;







	return 0;
}




