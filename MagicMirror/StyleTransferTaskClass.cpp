#define COMPILER_MSVC
#define NOMINMAX
#include <thread>
#include <vector>
#include "tensorflow/cc/ops/const_op.h"
#include "tensorflow/cc/ops/image_ops.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/util/command_line_flags.h"
#include "tensorflow/cc/ops/array_ops.h"
#include "StyleTransferTaskClass.h"
#include "Tools.h"

namespace InTheGlassRoom {
	StyleTransferTaskClass::StyleTransferTaskClass(const std::string name)
		:TaskClass(name)
	{
	}

	StyleTransferTaskClass::~StyleTransferTaskClass() {
		delete taskThread;
		taskThread = nullptr;
	}



	void StyleTransferTaskClass::stop() {
		this->canRun = false;
	}

	bool StyleTransferTaskClass::initTaskClass() {
		return true;
	}

	void StyleTransferTaskClass::setInputMsgQueue(MsgQueClass<std::vector<tensorflow::Tensor *>> *que) {
		this->inputMsgQue = que;
	}

	void StyleTransferTaskClass::setOutputMsgQueue(MsgQueClass<std::vector<tensorflow::Tensor>> *que) {
		this->outputMsgQue = que;
	}

	void StyleTransferTaskClass::mainLoop() {
		std::string Root_dir				= "D:/AI Art Project/tf-adain-master/tf-adain-master/input/";
		std::string Model_dir				= "D:/fronzen_model/";
		std::string graph_file				= "style_transfer_frozen_modified.pd";
		//std::string graph_file2 = "style_mean_std_frozen_modified.pd";
		std::string content_input_name		= "content_input:0";
		std::string style_input_name		= "style_input:0";
		std::string output_layer_name		= "transpose_1:0";
		std::string input_style_mean_name	= "style_mean_input:0";
		std::string input_style_std_name	= "style_std_input:0";
		std::string style_input_mean_name	= "moments/mean:0";
		std::string style_input_std_name	= "Sqrt:0";
		std::unique_ptr<tensorflow::Session>	style_session;
		std::string								graph_path1 = Model_dir + graph_file;
		unsigned int msg_code = 0;

		//load the style transfer graph
		tensorflow::Status load_graph_status = LoadGraph(graph_path1, &style_session);
		if (!load_graph_status.ok()) {
			LOG(ERROR) << load_graph_status;
			return;
		}
		else {
			LOG(INFO) << "Load style transfer graph success!";
		}
		unsigned int transfer_num = 0;
		std::vector<tensorflow::Tensor *> *foregroudTensor;
		//the main while loop of this task
		while (this->canRun) {
			//read the queue, if success
			if (status == 0) {
				std::this_thread::sleep_for(300ms);
				continue; 
			}
			if (this->inputMsgQue->recvMsg(msg_code, &foregroudTensor)) {
				//DWORD k = ::GetTickCount();
				//LOG(INFO) << "1th tensor " << foregroudTensor->at(0)->shape() << " 2th tensor " << foregroudTensor->at(1)->shape() << " 3th tensor " << foregroudTensor->at(2)->shape();
				std::vector<tensorflow::Tensor> *styled_outputs=new std::vector<tensorflow::Tensor>();
				tensorflow::Status run_status = style_session->Run({ { content_input_name,  *(foregroudTensor->at(0)) },{ input_style_mean_name,*(foregroudTensor->at(1)) },
				{ input_style_std_name,*(foregroudTensor->at(2)) } }, { output_layer_name }, {}, styled_outputs);
				delete foregroudTensor->at(0);
				foregroudTensor->at(0) = nullptr;
				delete foregroudTensor->at(1);
				foregroudTensor->at(1) = nullptr;
				delete foregroudTensor->at(2);
				foregroudTensor->at(2) = nullptr;
				foregroudTensor->clear();
				delete foregroudTensor;
				foregroudTensor = nullptr;
				if (!run_status.ok()) {
					LOG(ERROR) << "Running model failed: " << run_status;
				}
				else {
					this->outputMsgQue->sendMsg(msg_code, styled_outputs);
				}
				//LOG(INFO) << "totally time consum " << ::GetTickCount() - k;
			}
		}
		LOG(INFO) << "style transfer thread stop";
	}

	void StyleTransferTaskClass::run() {
		this->canRun = true;
		LOG(INFO) << "ready to run style transfer thread";
		//申明一个线程并运行它
		this->taskThread = new thread(&StyleTransferTaskClass::mainLoop, this);
	}
	void StyleTransferTaskClass::beforeChangeStatus() {
		this->inputMsgQue->clearMsg();
	}
}