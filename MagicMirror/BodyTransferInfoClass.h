#pragma once
#define COMPILER_MSVC
#define NOMINMAX
#ifndef _BODYTRANSFERINFOCLASS_H_
#define _BODYTRANSFERINFOCLASS_H_

#include<string>
#include<vector>
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
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/util/command_line_flags.h"
#include "tensorflow/cc/ops/array_ops.h"

namespace InTheGlassRoom{
	class BodyTransferInfoClass
	{
	public:
		BodyTransferInfoClass();
		~BodyTransferInfoClass();
		tensorflow::Tensor tensor;
		int styles[6];
	};
}
#endif

