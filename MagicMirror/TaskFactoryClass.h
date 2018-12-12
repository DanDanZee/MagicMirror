#pragma once
#ifndef _TASKFACTORYCLASS_H_
#define _TASKFACTORYCLASS_H_
#include <vector>
#include "TaskClass.h"
#include "MsgQueClass.h"

namespace InTheGlassRoom {
	class TaskClass;
	class TaskFactoryClass
	{
	public:
		TaskFactoryClass(string name);
		~TaskFactoryClass();
		void ChangeStatus(unsigned int status);
		void registeTask(TaskClass *task);
		void joinAllTask();
		void stopAllTask();
		void runAllTask();
		int  getTaskCount() const {
			return taskCount;
		}
	private:
		std::vector<TaskClass *> tasks;
		std::string				fname;
		int						taskCount;
	};
}
#endif // !_TASKFACTORYCLASS_H_