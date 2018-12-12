//------------------------------------------------------------------------------
//创建时间 2018-7-28
//
//
//------------------------------------------------------------------------------
#pragma once
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
#include<string>
namespace InTheGlassRoom {

	class PersonClass {

	public:
		PersonClass();
		~PersonClass();
		void			getStyle(tensorflow::Tensor &stylemean, tensorflow::Tensor &stylestd);	
		void			setTransitionNum(int num);
		void			ChangeStyle();
		void			setStyle(tensorflow::Tensor &mean, tensorflow::Tensor &std);
		bool			getInitStatus() const {
			return isInited;
		}
	private:
		tensorflow::Tensor	styleMeanTensor;
		tensorflow::Tensor	styleStdTensor;
		int					lastPtr = 0;
		int					curPtr = 0;
		int					frameCount=0;
		float				alpha = 0.05;
		int					transitionNum = 20;
		bool				transition=false;
		bool				isTraced = false;
		int					maxStyleIndex = 5;
		bool				isInited = false;
	};

};