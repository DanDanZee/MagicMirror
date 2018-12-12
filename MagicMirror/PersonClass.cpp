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
#include <string>
#include "PersonClass.h"
#include "Tools.h"

namespace InTheGlassRoom {

	PersonClass::PersonClass() {
		/*tensorflow::Tensor	tensor = tensorflow::Tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ 1,  lsheight, lswidth, lschannel }));*/
	}
	PersonClass::~PersonClass() {

	}

	void PersonClass::getStyle(tensorflow::Tensor &stylemean, tensorflow::Tensor &stylestd) {
		LOG(INFO) << "getting current style: " << curPtr;
		if (!isInited)return;
		if (this->transition) {
			//blending mean and std tensor
			float*	lastStyleMeanDataptr	= styleMeanTensor.Slice(lastPtr, lastPtr + 1).flat<float>().data();
			float*	lastStyleStdDataptr		= styleStdTensor.Slice(lastPtr, lastPtr + 1).flat<float>().data();
			float*	styleMeanDataptr		= styleMeanTensor.Slice(curPtr, curPtr + 1).flat<float>().data();
			float*	styleStdDataptr			= styleStdTensor.Slice(curPtr, curPtr + 1).flat<float>().data();
			float*	tempMeanPtr				= stylemean.flat<float>().data();
			float*	tempStdPtr				= stylestd.flat<float>().data();
			auto	width					= styleMeanTensor.dim_size(3);
			auto	height					= styleMeanTensor.dim_size(2);
			auto	channel					= styleMeanTensor.dim_size(1);
			for (int i = 0; i < width*height * channel; i++) {
				*tempMeanPtr = *lastStyleMeanDataptr *(1 - alpha*frameCount) + alpha*frameCount* *styleMeanDataptr;
				*tempStdPtr = *lastStyleStdDataptr *(1 - alpha*frameCount) + alpha*frameCount* *styleStdDataptr;
				lastStyleMeanDataptr++;
				lastStyleStdDataptr++;
				styleMeanDataptr++;
				styleStdDataptr++;
				tempMeanPtr++;
				tempStdPtr++;
			}
			LOG(INFO) << "Interplolation: " << frameCount << "-th frame";
			this->frameCount++;
			if (this->frameCount > this->transitionNum) {
				this->frameCount = 1;
				this->transition = false;
				this->lastPtr = this->curPtr;
			}
		}
		else {
			
			TensorClone(styleMeanTensor.Slice(curPtr, curPtr + 1), stylemean);
			TensorClone(styleStdTensor.Slice(curPtr, curPtr + 1), stylestd);
		}
	}
	void PersonClass::setTransitionNum(int num) {
		this->transitionNum = num;
		this->alpha = 1.0 / (float)num;
	}

	void PersonClass::ChangeStyle() {
		if (!isInited)return;
		this->transition = true;
		this->curPtr++;
		this->curPtr %= maxStyleIndex;
		LOG(INFO) << "Current style ptr:" << curPtr;
		//change style
	}
	void PersonClass::setStyle(tensorflow::Tensor &mean, tensorflow::Tensor &std) {
		this->styleMeanTensor = mean;
		this->styleStdTensor = std;
		isInited = true;
		LOG(INFO) << "Setting style";
	}
};