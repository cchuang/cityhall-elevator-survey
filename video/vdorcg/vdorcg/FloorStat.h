#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#ifdef __GNUC__
#include <opencv2/core/core.hpp>
#else
#include <opencv2/core.hpp>
#endif

#define ERR_CODE	-9999
#define NA_CODE		-9998
#define TYPE_GENERAL_CAR	1
#define TYPE_CAR_GROUP		2

const cv::Point trans_up(30, 7);
const cv::Point trans_down(30, 14);
const cv::Point trans_ii_up(51, 11);
const cv::Point trans_ii_down(31, 8);
const cv::Point trans_stop(45, 10);
const cv::Point trans_door(61, 11);
const cv::Point trans_car(87, 7);
const cv::Point trans_door_box(51, 0);
const cv::Size  size_door_box(18, 6);
#define	FLOOR_HEIGHT	(18.85)
#define	DOOR_SSTAT_NOT_HERE		0
#define	DOOR_SSTAT_OPEN			1
#define	DOOR_SSTAT_OPEN_H		2
#define	DOOR_SSTAT_CLOSED		3
class FloorStat {
	// height ~ 264/14 ~ 18.85
public: 
	FloorStat() {};
	FloorStat(int x, int y) : anchor(x, y) {}

	int	SetAnchor(int x, int y);
	int	RecogStat(cv::Mat	frame);
	void DetectDoorStat(cv::Mat	frame);

	bool	req_up;	
	bool	req_down;	
	bool	req_stop;
	bool	door_is_opening;
	bool	car_is_here;
	int		floor;
	int		type; // type I is for general car (elevator). type II is for car group.
	int		door_sstat;

private:
	cv::Point	anchor;
};
