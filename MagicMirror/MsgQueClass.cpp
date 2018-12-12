#include"MsgQueClass.h"

namespace InTheGlassRoom {
	template<typename T>
	MsgQueClass<T>::MsgQueClass(const string qName,  mutex *globalMutex)
		:msgQueueName(qName), msgMutex(globalMutex)
	{
	}
	template<typename T>
	MsgQueClass<T>::~MsgQueClass() {
	}

	//revice data from message queue
	template<typename T>
	bool MsgQueClass<T>::recvMsg(unsigned int &m_msg_code, T *p_msg) {
		bool result;
		Elements<T> queue_element;
		lock_guard<mutex> guard(*(this->msgMutex));

		if (this->msgQueue.empty()) {
			return false;
		}
		queue_element = this->msgQueue.front();
		this->msgQueue.pop_front();
		m_msg_code = queue_element.msgCode;
		p_msg = queue_element.pMessage;
		return true;
	}

	//send data to message queue
	template<typename T>
	bool MsgQueClass<T>::sendMsg(unsigned int m_msg_code, T *p_msg) {
		bool result;
		Elements<T> queue_element;
		queue_element.msgCode = m_msg_code;
		queue_element.pMessage = p_msg;
		lock_guard<mutex> guard(*(this->msgMutex));
		this->msgQueue.push_back(queue_element);
		return true;
	}
}