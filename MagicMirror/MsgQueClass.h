#pragma once
#ifndef _MSGQUECLASS_H_
#define _MSGQUECLASS_H_
#define COMPILER_MSVC
#define NOMINMAX
#include<deque>
#include<string>
#include<mutex>
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


namespace InTheGlassRoom {
	using namespace std;

	template<typename T>
	struct Elements{
		unsigned int	msgCode;
		T				*pMessage;
	};

	template<typename T>
	class MsgQueClass {
	public:
		MsgQueClass(const string qName,mutex *globalMutex) 
			:msgQueueName(qName), msgMutex(globalMutex)
		{
		}
		~MsgQueClass() {
			
		}

		//revice data from message queue
		template<typename T>
		bool		recvMsg(unsigned int &m_msg_code, T **p_msg)
		{
			bool result;
			Elements<T> queue_element;
			//lock_guard<mutex> lock(*(this->msgMutex));
			this->msgMutex->lock();
			if (this->msgQueue.empty()) {
				this->msgMutex->unlock();
				return false;
			}
			queue_element = this->msgQueue.front();
			this->msgQueue.pop_front();
			m_msg_code = queue_element.msgCode;
			*p_msg = queue_element.pMessage;
			this->msgMutex->unlock();
			return true;
		}

		//send data to message queue
		template<typename T>
		bool		sendMsg(unsigned int m_msg_code, T *p_msg) {
			bool result;
			Elements<T> queue_element;
			queue_element.msgCode = m_msg_code;
			queue_element.pMessage = p_msg;
			lock_guard<mutex> lock(*(this->msgMutex));
			//this->msgMutex->lock();
			this->msgQueue.push_back(queue_element);
			//this->msgMutex->unlock();
			return true;
		}

		void		clearMsg() {
			lock_guard<mutex> lock(*(this->msgMutex));
			this->msgQueue.clear();
			LOG(INFO) << "clear queue:" << this->msgQueueName;
		}

		bool		notEmpty() {
			lock_guard<mutex> lock(*(this->msgMutex));
			return this->msgQueue.empty();
		}

		const string getName(void) const {
			return msgQueueName;
		}

		const int getQueNum(void) const {
			return this->msgQueue.size();
		}

	private:
		string				msgQueueName;
		deque<Elements<T>>	msgQueue;
		mutex				*msgMutex;
	};
}
#endif // !_MSGQUECLASS_H_
