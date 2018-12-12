#include "facealign.h"

#define PI 3.1415926
//#define crop_patch_width 128
//#define crop_patch_height 128
//#define crop_eye_height 200
FaceAlign::FaceAlign()
{
    cv::Point2f desiredLeftEye=cv::Point2f(0.36,0.47);
    int desiredFaceWidth=128;
    int desiredFaceHeight=128;
    init(desiredLeftEye,desiredFaceWidth,desiredFaceHeight);
}

void FaceAlign::init(cv::Point2f desiredLeftEye,int desiredFaceWidth ,int desiredFaceHeight)
{
    desiredLeftEye_ =desiredLeftEye;
    desiredFaceWidth_ =desiredFaceWidth;
    desiredFaceHeight_ =desiredFaceHeight;
}
cv::Mat FaceAlign::align_OneFace(cv::Mat &OneFace ,std::vector<cv::Point2d> & landmark_oneface){
    vector<cv::Mat> vm = align_(OneFace , landmark_oneface);
    return vm[0];
}

cv::Mat FaceAlign::align(cv::Mat &Frame , std::vector<cv::Point2d> & landmarks){

    std::vector<cv::Mat>  aligned_face = align_(Frame,landmarks);
    return aligned_face[0];
}
std::vector<cv::Mat> FaceAlign::align_(cv::Mat &img , std::vector<cv::Point2d> &predictor)
{
    std::vector<cv::Point2d> predictor_ = predictor;
    int dY = predictor_.at(1).y - predictor_.at(0).y;
    int dX = predictor_.at(1).x - predictor_.at(0).x;
    double angle_ =angle(predictor_.at(0).x,predictor_.at(0).y,predictor_.at(1).x,predictor_.at(1).y);
    //std::cout << "angle" <<angle_ <<std::endl;
    float desiredRightEyeX = 1.0 - desiredLeftEye_.x;
    double dist = sqrt((dX * dX) + (dY * dY));
    float desiredDist = (desiredRightEyeX - desiredLeftEye_.x);
    desiredDist *= desiredFaceWidth_;
    float scale = desiredDist / dist;
    cv::Point2d eyesCenter = Point((predictor_.at(1).x + predictor_.at(0).x)/2,(predictor_.at(1).y + predictor_.at(0).y)/2);
    cv::Mat rotation_matrix = cv::getRotationMatrix2D(eyesCenter,angle_,scale);
    double tX = desiredFaceWidth_ *0.5;
    double tY = desiredFaceHeight_ * desiredLeftEye_.y;
    rotation_matrix.at<double>(0,2) += (tX - eyesCenter.x);
    rotation_matrix.at<double>(1,2) += (tY - eyesCenter.y);
    cv::Size desired_size = Size(desiredFaceWidth_,desiredFaceHeight_);
    cv::Mat output;
    cv::warpAffine(img,output,rotation_matrix,desired_size);

    std::vector<cv::Mat> output_patches;
    output_patches.push_back(output);
    return output_patches;
}
double FaceAlign::angle(int x1, int y1, int x2, int y2)
{
    double angle_temp;
    int xx, yy;

    xx = x2 - x1;
    yy = y2 - y1;

    if (xx == 0)
        angle_temp = 0;
    else
        angle_temp = atan(1.0*yy / xx);
    return (angle_temp*180/PI);
}
