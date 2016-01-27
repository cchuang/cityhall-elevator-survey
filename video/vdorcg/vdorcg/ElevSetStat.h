#pragma once
#include <vector>
#include <iostream>
#include <opencv2/core.hpp>
#include "ElevStat.h"

class ElevSetStat
{
public:
	ElevSetStat();
	~ElevSetStat();

	int	SetElev(cv::Point anchor, int num_floors, std::string name);
	int	SetElevGrp(cv::Point anchor, int num_floors, std::string name);
	int RecogStat(cv::Mat frame, double msec);

private:
	std::vector<ElevStat*>  elevs_stat;
	std::vector<ElevStat*>  elevgs_stat;
};

