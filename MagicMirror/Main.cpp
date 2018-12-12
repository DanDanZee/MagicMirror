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
	//�����ź���
	std::mutex detectdataMutex;
	std::mutex resultsMutex;
	std::mutex controlMutex;
	std::mutex infoMutex;
	
	//������ݶ���
	std::string qname = "detect data queue";
	InTheGlassRoom::MsgQueClass<std::vector<tensorflow::Tensor *>> detectdataQue(qname,&detectdataMutex);
	//����������
	std::string qname1 = "style transfer results queue";
	InTheGlassRoom::MsgQueClass<std::vector<tensorflow::Tensor>> resultsQue(qname1, &resultsMutex);
	//����ָ�����
	std::string qname2 = "control  queue";
	InTheGlassRoom::MsgQueClass<int> controlQue(qname2, &controlMutex);

	std::string qname3 = "mirror queue";
	InTheGlassRoom::MsgQueClass<cv::Mat> mirrorQue(qname3, &detectdataMutex);

	std::string qname4 = "body info queue";
	InTheGlassRoom::MsgQueClass<std::vector<InTheGlassRoom::BodyInfoClass>> bodyinfoQue(qname4, &infoMutex);
	//��ȡ�����ļ�
	//��ʼ�����з��
	//��ʼ��������
	//�򿪼���߳�
	//---�������
	//	|---�ض�����	---����indexһ����⵽���ᷢ���ı�
	//		|---�������
	//		|---��⶯��
	//		|---�������
	//		|---������
	
	//����task factory���������е�task
	std::string fname = "task monitor";
	InTheGlassRoom::TaskFactoryClass taskfactory(fname);
	//���Ǩ���߳�
	InTheGlassRoom::StyleTransferTaskClass styletransfer("style transfer task");
	styletransfer.setInputMsgQueue(&detectdataQue);
	styletransfer.setOutputMsgQueue(&resultsQue);
	//��������߳�
	InTheGlassRoom::BodyDetectTaskClass bodydetect("body detect task");
	if (!bodydetect.initSensor())
		return -1;
	bodydetect.setMsgQueue(&detectdataQue);
	bodydetect.setMirrorMsgQueue(&mirrorQue);
	bodydetect.setControlMsgQueue(&controlQue);
	bodydetect.setbodyinfoMsgQueue(&bodyinfoQue);
	
	//����ʾ�߳�
	InTheGlassRoom::DisplayTaskClass display("display task");
	display.setMsgQueue(&resultsQue);
	display.setMirrorMsgQueue(&mirrorQue);
	display.setcontrolMsgQueue(&controlQue);
	display.setbodyinfoMsgQueue(&bodyinfoQue);
	
	//ע����Ǩ��
	taskfactory.registeTask(&styletransfer);
	//ע��body���task
	taskfactory.registeTask(&bodydetect);
	//��task������ע���task
	taskfactory.registeTask(&display);
	//taskfactory.ChangeStatus(1);
	//run���е�task
	taskfactory.runAllTask();
	//��ֹ�����߳�
	taskfactory.stopAllTask();
	
	return 0;
}