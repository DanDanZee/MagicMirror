#pragma once
#ifndef _DISPLAYTASKCLASS_H_
#define _DISPLAYTASKCLASS_H_

#include<vector>
#include<string>
#include"TaskClass.h"
#include"MsgQueClass.h"
#include "BodyInfoClass.h"

namespace InTheGlassRoom {
	class DisplayTaskClass :public TaskClass {
	public:
		DisplayTaskClass(const std::string name);
		~DisplayTaskClass();
		//bool			stop();
		void			run();
		void			setMsgQueue(MsgQueClass<std::vector<tensorflow::Tensor>> *que);
		void			setMirrorMsgQueue(MsgQueClass<cv::Mat> *que);
		void			setcontrolMsgQueue(MsgQueClass<int> *que);
		void			setbodyinfoMsgQueue(MsgQueClass<vector<BodyInfoClass>> *que);
		void			beforeChangeStatus();
	private:
		bool			initTaskClass();
		virtual void	mainLoop();
		MsgQueClass<std::vector<tensorflow::Tensor>>	*msgQue;
		MsgQueClass<cv::Mat>							*mirrorMsgQue;
		MsgQueClass<int>								*controlMsgQue;
		MsgQueClass<vector<BodyInfoClass>>				*bodyinfoMsgQue;
		int				bgptr=0;
	};
}

#endif // !_DISPLAYTASKCLASS_H_