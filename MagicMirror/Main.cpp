#define COMPILER_MSVC
#define NOMINMAX
#include <iostream>
#include <string>
#include <fstream>
//#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include "json.hpp"
#include "MsgQueClass.h"
#include "BodyDetectTaskClass.h"
#include "tensorflow/core/platform/logging.h"
#include "StyleTransferTaskClass.h"
#include "DisplayTaskClass.h"
#include "TaskFactoryClass.h"
#include "BodyInfoClass.h"

// Kinect for Windows SDK Header

using json = nlohmann::json;

int main(int argc, char** argv)
{
	/*std::string tf_pb_file = "D:/fronzen_model/style_mean_std_frozen_modified.pd";
	cv::dnn::Net net=cv::dnn::readNetFromTensorflow(tf_pb_file);
	if(net.empty()){
		LOG(ERROR) << "read caffe model data failure...";
		return -1;
	}
	return 0;*/
	std::ifstream jsonConfigFileStream(".\\wocao.json", std::ifstream::in);
	json jsonConfigRoot;

	if (jsonConfigFileStream.is_open()) {
		jsonConfigFileStream >> jsonConfigRoot;
		LOG(INFO) << jsonConfigRoot["pi"];
	}
	//申明信号量
	std::mutex detectdataMutex;
	std::mutex resultsMutex;
	std::mutex controlMutex;
	std::mutex infoMutex;
	
	//检测数据队列
	std::string qname = "detect data queue";
	InTheGlassRoom::MsgQueClass<std::vector<tensorflow::Tensor *>> detectdataQue(qname,&detectdataMutex);
	//计算结果队列
	std::string qname1 = "style transfer results queue";
	InTheGlassRoom::MsgQueClass<std::vector<tensorflow::Tensor>> resultsQue(qname1, &resultsMutex);
	//控制指令队列
	std::string qname2 = "control  queue";
	InTheGlassRoom::MsgQueClass<int> controlQue(qname2, &controlMutex);

	std::string qname3 = "mirror queue";
	InTheGlassRoom::MsgQueClass<cv::Mat> mirrorQue(qname3, &detectdataMutex);

	std::string qname4 = "body info queue";
	InTheGlassRoom::MsgQueClass<std::vector<InTheGlassRoom::BodyInfoClass>> bodyinfoQue(qname4, &infoMutex);
	//读取配置文件
	//初始化所有风格
	//初始化传感器
	//打开检测线程
	//---检测人体
	//	|---特定人体	---人体index一旦检测到不会发生改变
	//		|---检测手势
	//		|---检测动作
	//		|---检测年龄
	//		|---检测表情
	
	//申明task factory来管理所有的task
	std::string fname = "task monitor";
	InTheGlassRoom::TaskFactoryClass taskfactory(fname);
	//风格迁移线程
	InTheGlassRoom::StyleTransferTaskClass styletransfer("style transfer task");
	styletransfer.setInputMsgQueue(&detectdataQue);
	styletransfer.setOutputMsgQueue(&resultsQue);
	//检测身体线程
	InTheGlassRoom::BodyDetectTaskClass bodydetect("body detect task");
	if (!bodydetect.initSensor())
		return -1;
	bodydetect.setMsgQueue(&detectdataQue);
	bodydetect.setMirrorMsgQueue(&mirrorQue);
	bodydetect.setControlMsgQueue(&controlQue);
	bodydetect.setbodyinfoMsgQueue(&bodyinfoQue);
	
	//打开显示线程
	InTheGlassRoom::DisplayTaskClass display("display task");
	display.setMsgQueue(&resultsQue);
	display.setMirrorMsgQueue(&mirrorQue);
	display.setcontrolMsgQueue(&controlQue);
	display.setbodyinfoMsgQueue(&bodyinfoQue);
	
	//注册风格迁移
	taskfactory.registeTask(&styletransfer);
	//注册body检测task
	taskfactory.registeTask(&bodydetect);
	//在task工厂中注册该task
	taskfactory.registeTask(&display);
	//taskfactory.ChangeStatus(1);
	//run所有的task
	taskfactory.runAllTask();
	//终止各个线程
	taskfactory.stopAllTask();
	
	return 0;
}