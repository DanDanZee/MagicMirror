#ifndef FACEALIGN_H
#define FACEALIGN_H

#include <opencv2/dnn.hpp>
#include <opencv2/dnn/shape_utils.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <set>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iterator>
#include <numeric>
#include <math.h>
using namespace cv;
using namespace std;

class FaceAlign {
public:
  FaceAlign();
  ~FaceAlign()
  {
  }
  void init(cv::Point2f desiredLeftEye,int desiredFaceWidth ,int desiredFaceHeight);
  std::vector<cv::Mat> align_(cv::Mat &img, std::vector<cv::Point2d> & predictor);
  cv::Mat align(cv::Mat &oneFace , std::vector<cv::Point2d> & landmarks);
  cv::Mat align_OneFace(cv::Mat &OneFace ,std::vector<cv::Point2d> & landmark_oneface);

private:

  cv::Point2d desiredLeftEye_;
  int desiredFaceWidth_;
  int desiredFaceHeight_;
  double angle(int x1, int y1, int x2, int y2);


};

#endif
