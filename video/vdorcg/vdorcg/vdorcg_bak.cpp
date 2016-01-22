/**
 * Elevator Video Recognizer
 * Maintainer: cc.huang@iis.sinica.edu.tw
*/
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>

using namespace std;
// Image Size: 1000x669
uchar Blue[585][2824];
uchar Green[585][2824];
uchar Red[585][2824];
uchar Gray[669][1000];

ofstream report("report.txt", std::ofstream::out /*| std::ofstream::app*/);
const int floor_number = 14;
string int2str(int &i) {
	string s;
	stringstream ss(s);
	ss << i;

	return ss.str();
}
class coordinate
{
 public:
	int x, y;
	coordinate(int x1, int y1)
	{
		(*this).x = x1;
		(*this).y = y1;
	}
	coordinate()
	{
	}
};
class Floor
{
 public:
	coordinate location;
	coordinate circle;
	int floor_num;
	bool is_waiting;
	bool is_open;
	bool in_this_floor;
	Floor()
	{

	}
};
class building
{

 public:
	Floor* floors;
	string name;
	building()
	{
		floors = new Floor[floor_number];
		for (int i = 0; i<14; i++)
		{
			floors[i].floor_num = i;
			floors[i].in_this_floor = false;
			floors[i].is_open = false;
			floors[i].is_waiting = false;
		}
	}

};
class point_rgb
{
 public: 
	int r, g, b;

	point_rgb(uchar r1, uchar g1, uchar b1)
	{
		(*this).r = static_cast<unsigned> (r1);
		(*this).g = static_cast<unsigned> (g1);
		(*this).b = static_cast<unsigned> (b1);

	}

};
void print_floor(int floor_num)
{
	if (floor_num >= 2)
		report << floor_num - 1 << "  floor  ";
	else if (floor_num == 0)
		report << " B2  floor  ";
	else if (floor_num == 1)
		report << " B1  floor  ";

}
point_rgb find_point_rgb(IplImage *Image1, coordinate cor)
{
	point_rgb rgb(Image1->imageData[cor.x*Image1->widthStep + cor.y * 3 + 2], Image1->imageData[cor.x*Image1->widthStep + cor.y * 3 + 1], Image1->imageData[cor.x*Image1->widthStep + cor.y * 3]);
	return rgb;
}
void print_rgb(point_rgb  rgb)
{
	cout << "red = " << rgb.r << endl;
	cout << "green = " << rgb.g << endl;
	cout << "blue = " << rgb.b << endl;
}
void Detect_floor_state(Floor & target_floor, IplImage *Image1, string building_name)
{
	/*if(target_floor.floor_num == 2)
	cout << "!!";*/
	point_rgb floor_color = find_point_rgb(Image1, target_floor.location), floor_circle = find_point_rgb(Image1, target_floor.circle);
	int average = (floor_color.r + floor_color.g + floor_color.b) / 3;

	if (floor_color.b >= 200 && floor_color.g >= 200 && floor_color.r >= 200) {
		// all white
		if (target_floor.is_open == false) {
			report << building_name;
			print_floor(target_floor.floor_num);
			report << "door is open" << endl;

		}
		target_floor.is_open = true;

	} else if (floor_color.b >= 200 && floor_color.g <= 50 && floor_color.r <= 50) {
		// blue
		if (target_floor.in_this_floor == false) {
			report << building_name;
			report << "Elevator is in  ";
			print_floor(target_floor.floor_num);
			report << endl;
		}
		target_floor.in_this_floor = true;


		if (target_floor.is_open == true) {
			report << building_name;
			print_floor(target_floor.floor_num);
			report << "door is close" << endl;
			target_floor.is_open = false;
		}

	} else if (floor_color.b < 150 && floor_color.g < 150 && floor_color.r < 150 && average < 150 && abs(average - floor_color.r)< 50 && abs(average - floor_color.g)< 50 && abs(average - floor_color.b)< 50) {
		// not in this floor
		if (target_floor.is_open == true) {
			report << building_name;
			print_floor(target_floor.floor_num);
			report << "door is close" << endl;
			target_floor.is_open = false;
		} if (target_floor.in_this_floor == true) {

		}
		target_floor.in_this_floor = false;

	}

	if (floor_circle.g >= 200) {
		// going up
		if (target_floor.is_waiting == false) {
			report << building_name;
			print_floor(target_floor.floor_num);
			report << "is waiting." << endl;

		}
		target_floor.is_waiting = true;

	} else {
		// nothing
		if (target_floor.is_waiting == true) {
		}
		target_floor.is_waiting = false;

	}
}



int image_handle(IplImage *Image1, building *b1) {

	//IplImage *Image1;
	// Image1 = cvLoadImage("elevator_sample1.jpg",1);
	//cvSaveImage("test.jpg",Image1);

	for (int i = 0; i < 14; i++)
	{
		Detect_floor_state((*b1).floors[i], Image1, (*b1).name);
	}


	//   cvNamedWindow("Gray Level",1);
	//   cvShowImage("Gray Level",Image1);
	//   //cvWaitKey(0);
	//point_rgb rgb1 = find_point_rgb(Image1, coordinate(331,827));
	//print_rgb(rgb1);
	//cvReleaseImage(&Image1);
	//   cvDestroyWindow("Gray Level");
	//system("pause");
	//report.close();
	return EXIT_SUCCESS;
}



int main(int argc, const char** argv)
{
	cv::Mat frame;
	IplImage* image;
	const char* in_file;

	if (argc < 2) {
		printf("Usage: %s <target file> \n", argv[0]);
		exit(1);
	}

	in_file = argv[1];
	printf("Input file is %s\n", in_file);

	CvCapture* capture = cvCreateFileCapture(in_file);
	/*
	cv::VideoCapture cap("C:\\Users\\601-2\\Documents\\208515270209_23.MP4");
	
	if (!cap.isOpened()) {
		std::cout << "Cannot open the video file on C++ API" << std::endl;
		return -1;
	}
	*/
	
	if (capture == NULL) {
		std::cout << "Cannot open the video file on C API" << std::endl;
		return -1;
	}
	building b1;
	b1.name = "Building NC 1's ";
	b1.floors[0].location = coordinate(605, 143);
	b1.floors[1].location = coordinate(588, 143);
	b1.floors[2].location = coordinate(568, 143);
	b1.floors[3].location = coordinate(547, 143);
	b1.floors[4].location = coordinate(532, 143);
	b1.floors[5].location = coordinate(513, 143);
	b1.floors[6].location = coordinate(494, 143);
	b1.floors[7].location = coordinate(472, 143);
	b1.floors[8].location = coordinate(455, 143);
	b1.floors[9].location = coordinate(437, 143);
	b1.floors[10].location = coordinate(416, 143);
	b1.floors[11].location = coordinate(400, 143);
	b1.floors[12].location = coordinate(380, 143);
	b1.floors[13].location = coordinate(360, 143);
	
	
	b1.floors[0].circle = coordinate(610, 121);
	b1.floors[1].circle = coordinate(589, 121);
	b1.floors[2].circle = coordinate(571, 121);
	b1.floors[3].circle = coordinate(551, 121);
	b1.floors[4].circle = coordinate(533, 121);
	b1.floors[5].circle = coordinate(515, 121);
	b1.floors[6].circle = coordinate(496, 121);
	b1.floors[7].circle = coordinate(478, 121);
	b1.floors[8].circle = coordinate(459, 121);
	b1.floors[9].circle = coordinate(439, 121);
	b1.floors[10].circle = coordinate(420, 121);
	b1.floors[11].circle = coordinate(402, 121);
	b1.floors[12].circle = coordinate(383, 121);
	b1.floors[13].circle = coordinate(366, 121);
	
	building b2;
	b2.name = "Building NC 2's ";
	b2.floors[0].location = coordinate(605, 267);
	b2.floors[1].location = coordinate(588, 267);
	b2.floors[2].location = coordinate(568, 267);
	b2.floors[3].location = coordinate(547, 267);
	b2.floors[4].location = coordinate(532, 267);
	b2.floors[5].location = coordinate(513, 267);
	b2.floors[6].location = coordinate(494, 267);
	b2.floors[7].location = coordinate(472, 267);
	b2.floors[8].location = coordinate(455, 267);
	b2.floors[9].location = coordinate(437, 267);
	b2.floors[10].location = coordinate(416, 267);
	b2.floors[11].location = coordinate(400, 267);
	b2.floors[12].location = coordinate(380, 267);
	b2.floors[13].location = coordinate(360, 267);
	
	
	b2.floors[0].circle = coordinate(612, 245);
	b2.floors[1].circle = coordinate(593, 245);
	b2.floors[2].circle = coordinate(575, 245);
	b2.floors[3].circle = coordinate(556, 245);
	b2.floors[4].circle = coordinate(537, 245);
	b2.floors[5].circle = coordinate(520, 245);
	b2.floors[6].circle = coordinate(501, 245);
	b2.floors[7].circle = coordinate(481, 245);
	b2.floors[8].circle = coordinate(462, 245);
	b2.floors[9].circle = coordinate(444, 245);
	b2.floors[10].circle = coordinate(424, 245);
	b2.floors[11].circle = coordinate(404, 245);
	b2.floors[12].circle = coordinate(386, 245);
	b2.floors[13].circle = coordinate(368, 245);
	
	building b3;
	b3.name = "Building NC 3's ";
	b3.floors[0].location = coordinate(605, 389);
	b3.floors[1].location = coordinate(588, 389);
	b3.floors[2].location = coordinate(568, 389);
	b3.floors[3].location = coordinate(547, 389);
	b3.floors[4].location = coordinate(532, 389);
	b3.floors[5].location = coordinate(513, 389);
	b3.floors[6].location = coordinate(494, 389);
	b3.floors[7].location = coordinate(472, 389);
	b3.floors[8].location = coordinate(455, 389);
	b3.floors[9].location = coordinate(437, 389);
	b3.floors[10].location = coordinate(416, 389);
	b3.floors[11].location = coordinate(400, 389);
	b3.floors[12].location = coordinate(380, 389);
	b3.floors[13].location = coordinate(360, 389);
	
	
	b3.floors[0].circle = coordinate(612, 369);
	b3.floors[1].circle = coordinate(593, 369);
	b3.floors[2].circle = coordinate(575, 369);
	b3.floors[3].circle = coordinate(556, 369);
	b3.floors[4].circle = coordinate(537, 369);
	b3.floors[5].circle = coordinate(520, 369);
	b3.floors[6].circle = coordinate(501, 369);
	b3.floors[7].circle = coordinate(481, 369);
	b3.floors[8].circle = coordinate(462, 369);
	b3.floors[9].circle = coordinate(444, 369);
	b3.floors[10].circle = coordinate(424, 369);
	b3.floors[11].circle = coordinate(404, 369);
	b3.floors[12].circle = coordinate(386, 369);
	b3.floors[13].circle = coordinate(368, 369);
	
	building b4;
	b4.name = "Building NC 4's ";
	b4.floors[0].location = coordinate(605, 515);
	b4.floors[1].location = coordinate(588, 515);
	b4.floors[2].location = coordinate(568, 515);
	b4.floors[3].location = coordinate(547, 515);
	b4.floors[4].location = coordinate(532, 515);
	b4.floors[5].location = coordinate(513, 515);
	b4.floors[6].location = coordinate(494, 515);
	b4.floors[7].location = coordinate(472, 515);
	b4.floors[8].location = coordinate(455, 515);
	b4.floors[9].location = coordinate(437, 515);
	b4.floors[10].location = coordinate(416, 515);
	b4.floors[11].location = coordinate(400, 515);
	b4.floors[12].location = coordinate(380, 515);
	b4.floors[13].location = coordinate(360, 515);
	
	
	b4.floors[0].circle = coordinate(612, 566);
	b4.floors[1].circle = coordinate(593, 566);
	b4.floors[2].circle = coordinate(575, 566);
	b4.floors[3].circle = coordinate(556, 566);
	b4.floors[4].circle = coordinate(537, 566);
	b4.floors[5].circle = coordinate(520, 566);
	b4.floors[6].circle = coordinate(501, 566);
	b4.floors[7].circle = coordinate(481, 566);
	b4.floors[8].circle = coordinate(462, 566);
	b4.floors[9].circle = coordinate(444, 566);
	b4.floors[10].circle = coordinate(424, 566);
	b4.floors[11].circle = coordinate(404, 566);
	b4.floors[12].circle = coordinate(386, 566);
	b4.floors[13].circle = coordinate(368, 566);
	
	building b5;
	
	b5.name = "Building NC 5's ";
	b5.floors[0].location = coordinate(605, 710);
	b5.floors[1].location = coordinate(588, 710);
	b5.floors[2].location = coordinate(568, 710);
	b5.floors[3].location = coordinate(547, 710);
	b5.floors[4].location = coordinate(532, 710);
	b5.floors[5].location = coordinate(513, 710);
	b5.floors[6].location = coordinate(494, 710);
	b5.floors[7].location = coordinate(472, 710);
	b5.floors[8].location = coordinate(455, 710);
	b5.floors[9].location = coordinate(437, 710);
	b5.floors[10].location = coordinate(416, 710);
	b5.floors[11].location = coordinate(400, 710);
	b5.floors[12].location = coordinate(380, 710);
	b5.floors[13].location = coordinate(360, 710);
	
	
	b5.floors[0].circle = coordinate(612, 691);
	b5.floors[1].circle = coordinate(593, 691);
	b5.floors[2].circle = coordinate(575, 691);
	b5.floors[3].circle = coordinate(556, 691);
	b5.floors[4].circle = coordinate(537, 691);
	b5.floors[5].circle = coordinate(520, 691);
	b5.floors[6].circle = coordinate(501, 691);
	b5.floors[7].circle = coordinate(481, 691);
	b5.floors[8].circle = coordinate(462, 691);
	b5.floors[9].circle = coordinate(444, 691);
	b5.floors[10].circle = coordinate(424, 691);
	b5.floors[11].circle = coordinate(404, 691);
	b5.floors[12].circle = coordinate(386, 691);
	b5.floors[13].circle = coordinate(368, 691);
	
	building b6;
	
	b6.name = "Building NC 6's ";
	b6.floors[0].location = coordinate(605, 911);
	b6.floors[1].location = coordinate(588, 911);
	b6.floors[2].location = coordinate(568, 911);
	b6.floors[3].location = coordinate(547, 911);
	b6.floors[4].location = coordinate(532, 911);
	b6.floors[5].location = coordinate(513, 911);
	b6.floors[6].location = coordinate(494, 911);
	b6.floors[7].location = coordinate(472, 911);
	b6.floors[8].location = coordinate(455, 911);
	b6.floors[9].location = coordinate(437, 911);
	b6.floors[10].location = coordinate(416, 911);
	b6.floors[11].location = coordinate(400, 911);
	b6.floors[12].location = coordinate(380, 911);
	b6.floors[13].location = coordinate(360, 911);
	
	
	b6.floors[0].circle = coordinate(612, 888);
	b6.floors[1].circle = coordinate(593, 888);
	b6.floors[2].circle = coordinate(575, 888);
	b6.floors[3].circle = coordinate(556, 888);
	b6.floors[4].circle = coordinate(537, 888);
	b6.floors[5].circle = coordinate(520, 888);
	b6.floors[6].circle = coordinate(501, 888);
	b6.floors[7].circle = coordinate(481, 888);
	b6.floors[8].circle = coordinate(462, 888);
	b6.floors[9].circle = coordinate(444, 888);
	b6.floors[10].circle = coordinate(424, 888);
	b6.floors[11].circle = coordinate(404, 888);
	b6.floors[12].circle = coordinate(386, 888);
	b6.floors[13].circle = coordinate(368, 888);
	for (int i = 0; i < 1; i++)
	{
		//cout << i << endl;
		image = cvQueryFrame(capture);
		//cap.read(frame);
		image_handle(image, &b1);
		image_handle(image, &b2);
		image_handle(image, &b3);
		image_handle(image, &b4);
		image_handle(image, &b5);
		image_handle(image, &b6);
		//cvShowImage("C API Image", image);
		//cv::imshow("C++ API Image", frame);
	
		//cv::waitKey( 10 ); 
		string name = "test" + int2str(i);
		name = name + ".jpg";
		//cvSaveImage(name.c_str(),image);
	}
	
	cvReleaseCapture(&capture);
	return 0;
}
