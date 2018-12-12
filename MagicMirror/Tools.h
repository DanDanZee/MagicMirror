#pragma once
#ifndef _TOOLS_H_
#define _TOOLS_H_
#define COMPILER_MSVC
#define NOMINMAX

#include<string>
#include<vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
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

#define STYLE_MEAN_STD_HEIGHT 800
#define STYLE_MEAN_STD_WIDTH STYLE_MEAN_STD_HEIGHT

namespace InTheGlassRoom {
	inline tensorflow::Status LoadGraph(const std::string& graph_file_name,
		std::unique_ptr<tensorflow::Session>* session) {
		tensorflow::GraphDef graph_def;
		tensorflow::Status load_graph_status =
			ReadBinaryProto(tensorflow::Env::Default(), graph_file_name, &graph_def);
		if (!load_graph_status.ok()) {
			return tensorflow::errors::NotFound("Failed to load compute graph at '",
				graph_file_name, "'");
		}
		session->reset(tensorflow::NewSession(tensorflow::SessionOptions()));
		tensorflow::Status session_create_status = (*session)->Create(graph_def);
		if (!session_create_status.ok()) {
			return session_create_status;
		}
		return tensorflow::Status::OK();
	}

	inline tensorflow::Tensor * Mat2Tensor_HWC(cv::Mat img_mat,int batch) {
		cv::cvtColor(img_mat, img_mat, cv::COLOR_BGR2RGB);
		cv::Mat img_mat_float;
		img_mat.convertTo(img_mat_float, CV_32FC3);
		img_mat_float /= 255.0;
		auto width = img_mat.cols;
		auto height = img_mat.rows;
		auto channel = img_mat.channels();
		// the net need a NCHW input
		tensorflow::Tensor * tensor=new tensorflow::Tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ batch,  height, width, channel }));
		auto tensor_mapped = tensor->tensor<float, 4>();
		const float* style_data = (float*)img_mat_float.data;
		for (int y = 0; y < height; ++y) {
			const float* source_row = style_data + (y * width * channel);
			for (int x = 0; x < width; ++x) {
				const float* source_pixel = source_row + (x * channel);
				for (int c = 0; c < channel; ++c) {
					const float* source_value = source_pixel + c;
					for(int b=0;b<batch;b++)
						tensor_mapped(b, y, x, c) = *source_value;
				}
			}
		}
		return tensor;
	}

	inline tensorflow::Tensor * Mat2Tensor_HWC(std::vector<cv::Mat> *img_mats) {
		// the net need a NCHW input
		int batch = img_mats->size();
		cv::Mat wori = img_mats->at(0);
		int height = wori.cols;
		int width = wori.rows;
		tensorflow::Tensor * tensor = new tensorflow::Tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ batch,  height, width, 3 }));
		for (int i = 0; i < batch; i++) {
			cv::Mat img_mat = img_mats->at(i);
			cv::cvtColor(img_mat, img_mat, cv::COLOR_BGR2RGB);
			cv::Mat img_mat_float;
			img_mat.convertTo(img_mat_float, CV_32FC3);
			img_mat_float /= 255.0;
			auto width = img_mat.cols;
			auto height = img_mat.rows;
			auto channel = img_mat.channels();
			auto tensor_mapped = tensor->tensor<float, 4>();
			const float* style_data = (float*)img_mat_float.data;
			for (int y = 0; y < height; ++y) {
				const float* source_row = style_data + (y * width * channel);
				for (int x = 0; x < width; ++x) {
					const float* source_pixel = source_row + (x * channel);
					for (int c = 0; c < channel; ++c) {
						const float* source_value = source_pixel + c;
						tensor_mapped(i, y, x, c) = *source_value;
					}
				}
			}
		}
		return tensor;
	}

	inline tensorflow::Tensor Mat2Tensor_HWC(const std::string& img_path) {
		cv::Mat img_mat = cv::imread(img_path);
		cv::cvtColor(img_mat, img_mat, cv::COLOR_BGR2RGB);
		cv::Mat img_mat_float;
		img_mat.convertTo(img_mat_float, CV_32FC3);
		img_mat_float = img_mat_float / 255.0;
		auto width = img_mat.cols;
		auto height = img_mat.rows;
		auto channel = img_mat.channels();
		// the net need a NCHW input
		tensorflow::Tensor tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ 1, height, width, channel }));
		auto tensor_mapped = tensor.tensor<float, 4>();

		const float* style_data = (float*)img_mat_float.data;
		for (int y = 0; y < height; ++y) {
			const float* source_row = style_data + (y * width * channel);
			for (int x = 0; x < width; ++x) {
				const float* source_pixel = source_row + (x * channel);
				for (int c = 0; c < channel; ++c) {
					const float* source_value = source_pixel + c;
					tensor_mapped(0, y, x, c) = *source_value;
				}
			}
		}
		return tensor;
	}

	inline tensorflow::Tensor * Mat2Tensor_HWC(const std::vector<std::string>* img_pathes) {

		int height = 1080;
		int width = 1920;
		int batch = img_pathes->size();
		tensorflow::Tensor * tensor = new tensorflow::Tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ batch,  height, width, 3 }));
		for (int i = 0; i < batch; i++) {
			cv::Mat img_mat = cv::imread(img_pathes->at(i));
			cv::resize(img_mat, img_mat, cv::Size(width, height));
			cv::cvtColor(img_mat, img_mat, cv::COLOR_BGR2RGB);
			cv::Mat img_mat_float;
			img_mat.convertTo(img_mat_float, CV_32FC3);
			img_mat_float = img_mat_float / 255.0;
			auto width = img_mat.cols;
			auto height = img_mat.rows;
			auto channel = img_mat.channels();
			// the net need a NCHW input
			auto tensor_mapped = tensor->tensor<float, 4>();
			const float* style_data = (float*)img_mat_float.data;
			for (int y = 0; y < height; ++y) {
				const float* source_row = style_data + (y * width * channel);
				for (int x = 0; x < width; ++x) {
					const float* source_pixel = source_row + (x * channel);
					for (int c = 0; c < channel; ++c) {
						const float* source_value = source_pixel + c;
						tensor_mapped(i, y, x, c) = *source_value;
					}
				}
			}
		}
		return tensor;
	}

	inline tensorflow::Tensor * ReadStyleImage2Tensor(const std::string  img_pathes[],const int num) {

		int height = STYLE_MEAN_STD_HEIGHT;
		int width = STYLE_MEAN_STD_WIDTH;
		int batch = num;
		tensorflow::Tensor * tensor = new tensorflow::Tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ batch,  height, width, 3 }));
		for (int i = 0; i < batch; i++) {
			cv::Mat img_mat = cv::imread(img_pathes[i]);
			cv::resize(img_mat, img_mat, cv::Size(width, height));
			cv::cvtColor(img_mat, img_mat, cv::COLOR_BGR2RGB);
			cv::Mat img_mat_float;
			img_mat.convertTo(img_mat_float, CV_32FC3);
			img_mat_float = img_mat_float / 255.0;
			auto width = img_mat.cols;
			auto height = img_mat.rows;
			auto channel = img_mat.channels();
			// the net need a NCHW input
			auto tensor_mapped = tensor->tensor<float, 4>();
			const float* style_data = (float*)img_mat_float.data;
			for (int y = 0; y < height; ++y) {
				const float* source_row = style_data + (y * width * channel);
				for (int x = 0; x < width; ++x) {
					const float* source_pixel = source_row + (x * channel);
					for (int c = 0; c < channel; ++c) {
						const float* source_value = source_pixel + c;
						tensor_mapped(i, y, x, c) = *source_value;
					}
				}
			}
		}
		return tensor;
	}

	inline void TensorClone(tensorflow::Tensor &scr, tensorflow::Tensor &dst) {
		auto				width = scr.dim_size(2);
		auto				height = scr.dim_size(1);
		auto				channel = scr.dim_size(0);
		float*				srcptr = scr.flat<float>().data();
		float*				dstptr = dst.flat<float>().data();
		for (int i = 0; i < width*height * channel; i++) {
			*dstptr = *srcptr;
			dstptr++;
			srcptr++;
		}
	}
}
#endif // !_TOOLS_H_
