#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <tesseract/baseapi.h>
#include <opencv2/core.hpp>

#define ERR_CODE	-9999
#define NA_CODE		-9998

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
	FloorStat(int x, int y);

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

const cv::Point trans_floor_box(0, 259);
// A text box which shows the elevator's name
const cv::Point trans_name_text_box(6, 0);
const cv::Size size_name_text_box(103, 17);
// A text box which shows which floor is in digit
const cv::Point trans_floor_text_box(77, 92);
const cv::Size size_floor_text_box(22, 21);
// A text box which shows the loading of this elevator in percentage
const cv::Point trans_weight_box(102, 104);
const cv::Size size_weight_box(12, 11);
// A box which shows the elevator is going up or down. 
const cv::Point trans_updown_box(53, 92);
const cv::Size size_updown_box(22, 21);
// A box which shows the OPEN button is pressed
const cv::Point trans_open_box(50, 74);
const cv::Size size_open_box(22, 11);
// A box which shows the service status
const cv::Point trans_service_box(1, 96);
const cv::Size size_service_box(22, 11);
#define TYPE_GENERAL_CAR	1
#define TYPE_CAR_GROUP		2

#define	GOING_UP	1
#define	GOING_DOWN	-1
#define	GOING_STOP	0
#define	GOING_UNKNOWN	-9
class ElevStat {
public:
	ElevStat(int x, int y, int num_floors, int highest);
	ElevStat(cv::Point ac, int num_floors, int highest);
	ElevStat(){};
	virtual ~ElevStat();

	int SetAnchor(int x, int y);
	int	SetNumFloors(int n, int highest);
	int SetType(int	in_type);
	int	RecogStat(cv::Mat	frame, double ts);
	int	Show(ElevStat *other, std::ostream &outfile);
	FloorStat *GetFS(int i);

	std::string	name;
	double	ts;
	int		type; // type I is for general car (elevator). type II is for car group.
	int		wh_floor;
	int		weight_percent;
	int		up_down; 
	bool	req_open;
	bool	stop_service;

private:
	cv::Point	anchor;
	std::vector<FloorStat*>  floors_stat;
	int	WriteRow(std::ostream &outfile, std::string event, std::string param1, std::string param2);
	int	WriteRow(std::ostream &outfile, std::string event, std::string param1);
	int	WriteRow(std::ostream &outfile, std::string event, int param1);
	int	WriteRow(std::ostream &outfile, std::string event, int param1, bool param2);
	int	WriteRow(std::ostream &outfile, std::string event, bool param1);
	int	WriteRow(std::ostream &outfile, std::string event);
	int	WriteRow(std::ostream &outfile, double ts, std::string event);
	bool CompReqStop(ElevStat *other);
	std::string WriteReqStop(void);
	int	RecogElevFloor(cv::Mat frame);
	int	RecogElevFloorByFloor(void);
	int RecogWeight(cv::Mat frame);
	int VerifyName(cv::Mat frame);
	void DetectDirection(cv::Mat frame);
	void DetectDoorOpen(cv::Mat frame);
	void DetectService(cv::Mat frame);
	char *RecogRectText(cv::Mat frame, cv::Rect roi, int ratio, bool debug);
};

static tesseract::TessBaseAPI *g_ocr;

