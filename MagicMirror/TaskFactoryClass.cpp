#include <vector>
#include "TaskFactoryClass.h"
#include "TaskClass.h"
namespace InTheGlassRoom {

	TaskFactoryClass::TaskFactoryClass(string name)
		:fname(name)
	{

	}

	TaskFactoryClass::~TaskFactoryClass(){
	}

	void TaskFactoryClass::ChangeStatus(unsigned int status) {
		for (int i = 0; i < taskCount; i++)
		{
			this->tasks[i]->beforeChangeStatus();
			this->tasks[i]->setStatus(status);
		}
	}
	void TaskFactoryClass::registeTask(TaskClass * task) {
		this->tasks.push_back(task);
		task->setFactory(this);
		taskCount++;
	}

	void TaskFactoryClass::joinAllTask() {
		for (int i = 0; i < taskCount; i++)
		{
			this->tasks[i]->taskWait();
		}
	}

	void TaskFactoryClass::stopAllTask() {
		for (int i = 0; i < taskCount; i++)
		{
			this->tasks[i]->stop();
		}
		this->joinAllTask();
	}
	void TaskFactoryClass::runAllTask() {
		for (int i = 0; i < taskCount; i++)
		{
			this->tasks[i]->run();
		}
	}
}