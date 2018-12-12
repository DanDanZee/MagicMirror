#pragma once
#ifndef _BODYINFOCLASS_H_
#define _BODYINFOCLASS_H_

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


namespace InTheGlassRoom {

	class BodyInfoClass
	{
	public:
		BodyInfoClass();
		BodyInfoClass(int x, int y, cv::Mat *mask);
		~BodyInfoClass();
		int x;
		int y;
		int width;
		int height;
		cv::Mat *mask;
		cv::Mat *face;
	};
}
#endif
