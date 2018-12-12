#include<thread>
#include"TaskClass.h"
namespace InTheGlassRoom {
	TaskClass::TaskClass(const std::string name)
		:task_class_name(name)
	{
	}

	TaskClass::~TaskClass() {

	}

	void TaskClass::run() {
		this->canRun = true;
		//申明一个线程并运行它
		/*std::thread taskthread(&TaskClass::mainLoop,this);*/
		this->mainLoop();
	}

	void TaskClass::stop() {
		this->canRun = false;
	}

	bool TaskClass::initTaskClass() {
		return true;
	}

	void TaskClass::setStatus(unsigned int code) {
		this->status = code;
	}

	void TaskClass::mainLoop() {

	}
	void TaskClass::setFactory(TaskFactoryClass *factory) {
		this->factory = factory;
	}

	void TaskClass::beforeChangeStatus() {

	}

	void TaskClass::taskWait() {
		if (this->taskThread != nullptr) {
			this->taskThread->join();
		}
		else {
			LOG(ERROR) << "the task thread is null!";
		}
	}
}