#include <iostream>
#include <stack>
#include <stdlib.h>
#include <vector>
#include <algorithm>

using namespace std;


struct Point
{
	double x, y;

};
std::vector <Point> estimatedPoints;


//needed for  sorting points with referenceto  the first point 
Point p0;

// A utility function to find next to top in a stack
Point nextToTop(stack<Point> &S)
{
	Point p = S.top();
	S.pop();
	Point res = S.top();
	S.push(p);
	return res;
}

// A utility function to swap two points
void swap(Point &p1, Point &p2)
{
	Point temp = p1;
	p1 = p2;
	p2 = temp; 
}

// A utility function to return square of distance
// between p1 and p2
double distSq(Point p1, Point p2)
{
	return (p1.x - p2.x)*(p1.x - p2.x) +
		(p1.y - p2.y)*(p1.y - p2.y);
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(Point p, Point q, Point r)
{
	double val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	if (val == 0) return 0;  // colinear
	return (val > 0) ? 1 : 2; // clock or counterclock wise
}

// A function used by library function qsort() to sort an array of
// points with respect to the first point
int compare(const void *vp1, const void *vp2)
{
	Point *p1 = (Point *)vp1;
	Point *p2 = (Point *)vp2;

	// Find orientation
	int o = orientation(p0, *p1, *p2);
	if (o == 0)
		return (distSq(p0, *p2) >= distSq(p0, *p1)) ? -1 : 1;

	return (o == 2) ? -1 : 1;
}

// Prints convex hull of a set of n points.
std::vector<Point> convexHull(std::vector<Point> points, int n)
{
	std::vector <Point> output;
	// Find the bottommost point
	double ymin = points[0].y, min = 0;
	for (int i = 1; i < n; i++)
	{
		double y = points[i].y;

		// Pick the bottom-most or chose the left
		// most point in case of tie
		if ((y < ymin) || (ymin == y &&
			points[i].x < points[min].x))
			ymin = points[i].y, min = i;
	}

	// Place the bottom-most point at first position
	swap(points[0], points[min]);

	// Sort n-1 points with respect to the first point.
	// A point p1 comes before p2 in sorted ouput if p2
	// has larger polar angle (in counterclockwise
	// direction) than p1
	p0 = points[0];
	qsort(&points[1], n - 1, sizeof(Point), compare);

	// If two or more points make same angle with p0,
	// Remove all but the one that is farthest from p0
	// Remember that, in above sorting, our criteria was
	// to keep the farthest point at the end when more than
	// one points have same angle.
	int m = 1; // Initialize size of modified array
	for (int i = 1; i<n; i++)
	{
		// Keep removing i while angle of i and i+1 is same
		// with respect to p0
		while (i < n - 1 && orientation(p0, points[i],
			points[i + 1]) == 0)
			i++;


		points[m] = points[i];
		m++;  // Update size of modified array
	}




	// Create an empty stack and push first three points
	// to it.
	stack<Point> S;
	S.push(points[0]);
	S.push(points[1]);
	S.push(points[2]);

	// Process remaining n-3 points
	for (int i = 3; i < m; i++)
	{
		// Keep removing top while the angle formed by
		// points next-to-top, top, and points[i] makes
		// a non-left turn
		while (orientation(nextToTop(S), S.top(), points[i]) != 2)
			S.pop();
		S.push(points[i]);
	}
	int a = 0;
	// Now stack has the output points, print contents of stack
	while (!S.empty())
	{
		Point p = S.top();

		output.push_back(p);

		S.pop();
	}
	return output;
}



// Define Infinite (Using INT_MAX caused overflow problems)
#define INF 10000000



// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool onSegment(Point p, Point q, Point r)
{
	if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
		q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
		return true;
	return false;
}



// The function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool doIntersect(Point p1, Point q1, Point p2, Point q2)
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

// Returns true if the point p lies inside the polygon[] with n vertices
bool isInside(std::vector<Point> polygon, int n, Point p)
{
	// There must be at least 3 vertices in polygon[]
	if (n < 3)  return false;

	// Create a point for line segment from p to infinite
	Point extreme = { INF, p.y };

	// Count intersections of the above line with sides of polygon
	int count = 0, i = 0;
	do
	{
		int next = (i + 1) % n;

		// Check if the line segment from 'p' to 'extreme' intersects
		// with the line segment from 'polygon[i]' to 'polygon[next]'
		if (doIntersect(polygon[i], polygon[next], p, extreme))
		{
			// If the point 'p' is colinear with line segment 'i-next',
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









Point getCenter(std::vector<Point> points)
{
	Point center;
	double xtotal = 0;
	double ytotal = 0;
	double masstotal = 0;
	int i;
	for (i = 0; i<points.size(); i++)
	{
		xtotal += points[i].x;
		ytotal += points[i].y;

	}
	masstotal = points.size();
		//cout << masstotal <<"\n";
	center.x = xtotal / masstotal;
	center.y = ytotal / masstotal;

	//cout << masstotal;
	return center;

}


void floodFill(Point current, double interval, std::vector<Point> sensorLoc)
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
		return;
	}
	//up
	Point p;
	p = current;
	p.y = p.y + interval;
	floodFill(p, interval, sensorLoc);
	//right
	p = current;
	p.x = p.x + interval;
	floodFill(p, interval, sensorLoc);

	//left
	p = current;
	p.x = p.x - interval;
	floodFill(p, interval, sensorLoc);
	//down
	p = current;
	p.y = p.y - interval;
	floodFill(p, interval, sensorLoc);
	return;
}







//name get points later
int main()
{

	std::vector<Point> points = { { 28.600367,-81.198041 } ,{ 28.600693, -81.198229 } ,{ 28.600893, -81.197376 } ,{ 28.600700, -81.197134 } };
	//std::vector<Point> points = { { 0,0 } ,{ 0, 10 } ,{ 10, 10} ,{ 10, 0 } };
	Point center = getCenter(points);
	//cout << "(" << std::fixed<< center.x << "," << center.y << ")\n\n";




	double interval = .0005;
	floodFill(center, interval, points);
	unsigned int i;
	for (i = 0; i<estimatedPoints.size(); i++)
	{
		estimatedPoints.at(i).x = estimatedPoints.at(i).x ;
		estimatedPoints.at(i).y = estimatedPoints.at(i).y ;
		cout << "(" << std::fixed << estimatedPoints.at(i).x << "," << estimatedPoints.at(i).y << ")\n";
	}
	//wait for input so the command prompt doesn't close
	int x;
	cin >> x;
	return 0;
}