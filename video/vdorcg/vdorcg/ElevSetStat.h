#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#ifdef __GNUC__
#include <opencv2/core/core.hpp>
#else
#include <opencv2/core.hpp>
#endif
#include "ElevStat.h"

class ElevSetStat
{
public:
	ElevSetStat();
	~ElevSetStat();

	int	SetElev(cv::Point anchor, int num_floors, int highest, std::string name);
	int	SetElevGrp(cv::Point anchor, int num_floors, int highest, std::string name);
	int RecogStat(cv::Mat frame, double ts);
	int	Show(ElevSetStat *other, std::ostream &outfile);
	ElevStat *GetES(int idx); 
	ElevStat *GetEGS(int idx); 

private:
	std::vector<ElevStat*>  elevs_stat;
	std::vector<ElevStat*>  elevgs_stat;
};

