#pragma once
#include <vector>
#include <iostream>
#include <tesseract/baseapi.h>
#include <opencv2/core.hpp>

const cv::Point trans_up(30, 7);
const cv::Point trans_down(30, 14);
const cv::Point trans_ii_up(51, 11);
const cv::Point trans_ii_down(31, 8);
const cv::Point trans_stop(45, 10);
const cv::Point trans_door(61, 11);
const cv::Point trans_car(87, 7);
#define	FLOOR_HEIGHT	(18.85)
#define TYPE_GENEARL_CAR	1
#define TYPE_CAR_GROUP		2

class FloorStat {
	// height ~ 264/14 ~ 18.85
public: 
	FloorStat() {};
	FloorStat(int x, int y);

	int	SetAnchor(int x, int y);
	int	RecogStat(cv::Mat	frame);

	bool	req_up;	
	bool	req_down;	
	bool	req_stop;
	bool	door_is_opening;
	bool	car_is_here;
	int		floor;
	int		type; // type I is for general car (elevator). type II is for car group.

private:
	cv::Point	anchor;
};

const cv::Point trans_floor_box(0, 259);
const cv::Point trans_floor_text_box(77, 92);
const cv::Size size_floor_text_box(22, 21);
const cv::Point trans_weight_box(102, 104);
const cv::Size size_weight_box(12, 11);
#define	FLOOR_START_AT	12
class ElevStat {
public:
	ElevStat(int x, int y, int num_floors);
	ElevStat(cv::Point ac, int num_floors);
	ElevStat(){};
	virtual ~ElevStat();

	int SetAnchor(int x, int y);
	int	SetNumFloors(int n);
	int SetType(int	in_type);
	int	RecogStat(cv::Mat	frame, double dmsec);
	int	Show();

	std::string	name;
	double	msec;
	int		wh_floor;
	int		weight_percent;
	int		type; // type I is for general car (elevator). type II is for car group.

private:
	cv::Point	anchor;
	std::vector<FloorStat*>  floors_stat;
	int	RecogElevFloor(cv::Mat frame);
	int RecogWeight(cv::Mat frame);
	char *RecogRect(cv::Mat frame, cv::Rect roi, bool debug);
};

static tesseract::TessBaseAPI *g_ocr;
