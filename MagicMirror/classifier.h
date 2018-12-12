#ifndef CLASSIFIER_H
#define CALSSIFIER_H

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


using namespace cv;
using namespace std;

#define mydataFmt float
#define NumPoint   4


class Classifier {
 public:
    cv::CascadeClassifier d_face;
    Classifier();
    ~Classifier(){
    }
    void init(std::string &model,std::string &weight);
    std::vector<cv::Mat > classify(cv::Mat img,int desired_width);
    std::vector<cv::Mat > classify_landmark(cv::Mat img,int desired_width);
    void PrintM(Mat A);
    int getAge(cv::Mat img , int desired_width);
    cv::Mat cropRandom(cv::Mat img);
    int getAge_fromAligned(cv::Mat Aligned_face , int desired_width);

 private:
  cv::Ptr<cv::dnn::Net> net_;

  bool init_flag;
  std::vector<std::vector<string> > labels_;
};

#endif
