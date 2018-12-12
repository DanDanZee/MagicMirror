#pragma once
#ifndef _TASKCLASS_H_
#define _TASKCLASS_H_
#include <string>
#include <thread>
#include "TaskFactoryClass.h"

namespace InTheGlassRoom {
	class TaskFactoryClass;
	class TaskClass {
	public:
		TaskClass(const std::string name);
		~TaskClass();
		virtual void		run();
		void				stop();
		unsigned int		getStatus() const {
			return this->status;
		}
		void				setStatus(unsigned int code);
		void				setFactory(TaskFactoryClass *factory);
		virtual void		beforeChangeStatus();
		void				taskWait();
	protected:
		virtual bool		initTaskClass();
		virtual void		mainLoop();
		//the status of this task
		unsigned int		status=0;
		std::string			task_class_name;
		bool				canRun=false;
		TaskFactoryClass	*factory;
		std::thread			*taskThread=nullptr;
	};
};

#endif // !_TASKCLASS_H_