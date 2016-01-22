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

	int	SetElev(Point anchor, int num_floors, string name);
	int RecogStat(Mat frame, double msec);

private:
	vector<ElevStat*>  elevs_stat;
};

