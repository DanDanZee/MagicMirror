#include "classifier.h"

Classifier::Classifier()
{
    std::string model_file = "./models/deploy.prototxt";
    std::string trained_file ="./models/weights.caffemodel";
    init( model_file, trained_file);
}
void Classifier::init(std::string &model,std::string &weight)
{
    if(!d_face.load("./models/haarcascade_frontalface_alt2.xml"))
    {
        std::cout << "load file failed!"<<std::endl;
        exit(-1);
    }
    net_ = new cv::dnn::Net;
    (*net_) = cv::dnn::readNetFromCaffe(model, weight);
    net_->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    if (net_->empty())
    {
        exit(-1);
    }

    std::cout<< "load labels success" << std::endl;
    init_flag = true;
}


cv::Mat Classifier::cropRandom(cv::Mat img){
    int margin = 0.05 * (img.rows + img.cols);
    int x1 = rand() % margin;
    int y1 = rand() % margin;
    int x2 = img.rows - rand() % margin;
    int y2 = img.cols - rand() % margin;

    cv::Rect rect = cv::Rect(x1 ,y1 ,x2 - x1 , y2 - y1);
    rect = rect & Rect(0,0,img.rows ,img.cols);
    return img(rect);
}



int Classifier::getAge_fromAligned(cv::Mat Aligned_face , int desired_width){
    cv::Mat inputBlob = cv::dnn::blobFromImage(Aligned_face, 1, cv::Size(desired_width,desired_width), cv::Scalar(0,0,0), false);
    net_->setInput(inputBlob,"data");
    std::vector< cv::Mat > outputs;
    std::vector< cv::String > names={"probs"};


//#define test_time_cost
#ifdef test_time_cost
    cout<<"start testing time cost of classification"<<endl;
    clock_t start , end;
    start = clock();
    for(int i = 0;i< 10000;i++){
        net_->forward(outputs,names);
    }
    end = clock();
    cout<<"time : "<<(double)(end - start)*1000/CLOCKS_PER_SEC<<" ms"<<endl;
    exit(-1);
#else
    net_->forward(outputs,names);
#endif

    cv::Mat predict_age = outputs[0];
    vector<float > Conf;
    Conf.push_back(predict_age.at<float>(0,0));
    Conf.push_back(predict_age.at<float>(0,1));
    Conf.push_back(predict_age.at<float>(0,2));
    Conf.push_back(predict_age.at<float>(0,3));
    float age = 0;
    age += Conf[0] * 6;
    age += Conf[1] * 23;
    age += Conf[2] * 37;
    age += Conf[3] * 64;

    return int(age);
}

