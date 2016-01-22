#pragma once
#include <vector>
#include <iostream>
#include <opencv2/core.hpp>

using namespace std;
using namespace cv;	

/*
class Coordinate {
 public:
	int x, y;
	Coordinate(int x, int y); 
	Coordinate() {}; 
};
*/

const Point trans_up(30, 7);
const Point trans_down(30, 14);
const Point trans_stop(45, 10);
const Point trans_door(61, 11);
const Point trans_car(87, 7);
#define	FLOOR_HEIGHT	(18.85)
class FloorStat {
	// height ~ 264/14 ~ 18.85
public: 
	FloorStat() {};
	FloorStat(int x, int y);

	int	SetAnchor(int x, int y);
	int	RecogStat(Mat	frame);

	bool	req_up;	
	bool	req_down;	
	bool	req_stop;
	bool	door_is_opening;
	bool	car_is_here;
	int		floor;

private:
	Point	anchor;
};

const Point trans_floor_box(0, 259);
const Point trans_floor_text(75, 92);
#define	FLOOR_START_AT	12
class ElevStat {
public:
	ElevStat(int x, int y, int num_floors);
	ElevStat(Point ac, int num_floors);
	ElevStat(){};
	virtual ~ElevStat();

	int SetAnchor(int x, int y);
	int	SetNumFloors(int n);
	int	RecogStat(Mat	frame, double dmsec);
	int	Show();
	string	name;
	double	msec;
	int		wh_floor;

private:
	Point	anchor;
	vector<FloorStat*>  floors_stat;
	int	RecogElevFloor(Mat frame);
};

