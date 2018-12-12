#define COMPILER_MSVC
#define NOMINMAX
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "DisplayTaskClass.h"
#include "tensorflow/core/platform/logging.h"
#include "Windows.h"
#include "BodyInfoClass.h"

#define MAX_FRAME_COUNT 10

namespace InTheGlassRoom {
	DisplayTaskClass::DisplayTaskClass(const string name)
		:TaskClass(name)
	{

	}

	DisplayTaskClass::~DisplayTaskClass() {

	}

	void DisplayTaskClass::run() {
		this->canRun = true;
		//申明一个线程并运行它
		this->mainLoop();
		//thread taskthread(&DisplayTaskClass::mainLoop, this);
	}

	//bool DisplayTaskClass::stop() {

	//}
	bool DisplayTaskClass::initTaskClass() {
		return true;
	}

	void DisplayTaskClass::setMsgQueue(MsgQueClass<std::vector<tensorflow::Tensor>> *que) {
		this->msgQue = que;
	}

	void DisplayTaskClass::setMirrorMsgQueue(MsgQueClass<cv::Mat> *que) {
		this->mirrorMsgQue = que;
	}

	void DisplayTaskClass::setcontrolMsgQueue(MsgQueClass<int> *que) {
		this->controlMsgQue = que;
	}

	void DisplayTaskClass::setbodyinfoMsgQueue(MsgQueClass<vector<BodyInfoClass>> *que) {
		this->bodyinfoMsgQue = que;
	}

	void DisplayTaskClass::mainLoop() {
		std::vector<tensorflow::Tensor> *outputs = nullptr;
		std::vector<BodyInfoClass> *infos = nullptr;
		std::string bgimgs[] = {
			"D:\\style\\bg\\3.jpg","D:\\style\\bg\\938.jpg",
			"D:\\style\\bg\\946.jpg","D:\\style\\bg\\984.jpg",
			"D:\\style\\bg\\9023.jpg","D:\\style\\bg\\9088.jpg",
			"D:\\style\\bg\\9103.jpg","D:\\style\\bg\\9177.jpg",
			"D:\\style\\bg\\9561.jpg","D:\\style\\bg\\96.jpg"
		};
		cv::Mat *mirrorOutput = nullptr;
		cv::Mat BGI_temp=cv::imread(bgimgs[bgptr]);
		cv::Mat BGI;
		cv::Mat LastBG;
		cv::resize(BGI_temp, BGI_temp, cv::Size(1920, 1080));
		BGI = BGI_temp.clone();
		int *cmd;
		unsigned int msg_code = 0;
		bool is_change = false;
		float alpha = 0;
		int frame_count = 0;
		cv::namedWindow("Styletransfer", CV_WINDOW_NORMAL);
		cv::setWindowProperty("Styletransfer", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
		while (this->canRun) {
			//DWORD k = ::GetTickCount();
			if (this->controlMsgQue->recvMsg(msg_code, &cmd)) {
				switch (*cmd) {
				case 1: {
					LOG(INFO) << "get control cmd 1";
					bgptr++;
					bgptr %= 9;
					LastBG = BGI.clone();
					BGI_temp = cv::imread(bgimgs[bgptr]);
					is_change = true;
					cv::resize(BGI_temp, BGI_temp, cv::Size(1920, 1080));
					break;
				}
						// mirror status
				case 2: {
					LOG(INFO) << "get control cmd 2";
					this->factory->ChangeStatus(0);
					break;
				}
				case 3: {
					LOG(INFO) << "get control cmd 3";
					this->factory->ChangeStatus(1);
					break;
				}
				case 4: {
					LOG(INFO) << "get control cmd 4";
					bgptr++;
					bgptr %= 9;
					LastBG = BGI.clone();
					BGI_temp = cv::imread(bgimgs[bgptr]);
					is_change = true;
					cv::resize(BGI_temp, BGI_temp, cv::Size(1920, 1080));
					break;
				}
				}
				delete cmd;
				cmd = nullptr;
			}
			
			if (status == 0) {
				if (this->mirrorMsgQue->recvMsg(msg_code, &mirrorOutput)) {
					cv::imshow("Styletransfer", *mirrorOutput);
					delete mirrorOutput;
				}
			}
			else {
				
				if (this->msgQue->recvMsg(msg_code, &outputs)) {
					cv::Mat tempBG	=	BGI.clone();
					float*	dataptr	=	(*outputs)[0].flat<float>().data();
					auto	width	=	(*outputs)[0].dim_size(2);
					auto	height	=	(*outputs)[0].dim_size(1);
					//LOG(INFO) << "transfer msg code: " << msg_code;
					if (this->bodyinfoMsgQue->recvMsg(msg_code, &infos)) {
						if (is_change) {
							cv::addWeighted(LastBG,  1 - alpha, BGI_temp, alpha, 0.0, BGI);
							alpha += 0.1;
							frame_count++;
							if (frame_count > MAX_FRAME_COUNT) {
								alpha = 0;
								frame_count = 0;
								is_change = false;
							}
						}
						//LOG(INFO) << "mask msg code: " << msg_code;
						int people = infos->size();
						//convert tensors into cv::Mats
						int yoffset = 0;
						for (int i = 0; i < people; i++) {
							BodyInfoClass info = infos->at(i);
							cv::Mat res_img(height, width, CV_32FC3, dataptr + i*height*width * 3);
							res_img.convertTo(res_img, CV_8UC3, 255);
							cv::resize(res_img, res_img, cv::Size(info.width, info.height));
							cv::cvtColor(res_img, res_img, cv::COLOR_RGB2BGR);
							cv::Rect rect_temp(info.x, info.y, info.width, info.height);
							res_img.copyTo(tempBG(rect_temp), *(info.mask));
							cv::Rect temprect = cv::Rect(0, yoffset, info.face->cols, info.face->rows);
							yoffset += info.face->rows;
							info.face->copyTo(tempBG(temprect));
							delete info.mask;
							info.mask = nullptr;
							//delete &(infos->at(i));
						}
						infos->clear();
						delete infos;
						infos = nullptr;
					}
					//delete dataptr;
					outputs->clear();
					dataptr = nullptr;
					delete outputs;
					outputs = nullptr;
					
					cv::imshow("Styletransfer", tempBG);
				}
			}
			//LOG(INFO) << " display totally time consum " << ::GetTickCount() - k;
			//rec control cmd
			
			if (cv::waitKey(1) == VK_ESCAPE) {
				this->stop();
			}
		}
	}

	void DisplayTaskClass::beforeChangeStatus() {
		//clear all input queue
		this->mirrorMsgQue->clearMsg();
		this->msgQue->clearMsg();
		this->bodyinfoMsgQue->clearMsg();
	}
}