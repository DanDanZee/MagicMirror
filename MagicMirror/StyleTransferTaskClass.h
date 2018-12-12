#pragma once
#ifndef _STYLETRANSFERTASKCLASS_H_
#define _STYLETRANSFERTASKCLASS_H_

#include<string>
#include<vector>
#include"TaskClass.h"
#include"MsgQueClass.h"
#include "BodyInfoClass.h"

namespace InTheGlassRoom {
	class StyleTransferTaskClass :public TaskClass {
	public:
		StyleTransferTaskClass(const std::string name);
		~StyleTransferTaskClass();
		void			run();
		void			stop();
		void			setInputMsgQueue(MsgQueClass<std::vector<tensorflow::Tensor *>> *que);
		void			setOutputMsgQueue(MsgQueClass<std::vector<tensorflow::Tensor>> *que);
		void			beforeChangeStatus();
	private:
		bool			initTaskClass();
		virtual void	mainLoop();
		MsgQueClass<std::vector<tensorflow::Tensor *>>				 *inputMsgQue;
		MsgQueClass<std::vector<tensorflow::Tensor>> *outputMsgQue;
	};
}

#endif // !_STYLETRANSFERTASKCLASS_H_
