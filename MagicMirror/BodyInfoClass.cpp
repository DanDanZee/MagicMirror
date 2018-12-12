#include "BodyInfoClass.h"
namespace InTheGlassRoom {

	BodyInfoClass::BodyInfoClass()
	{
	}

	BodyInfoClass::BodyInfoClass(int inx,int iny,cv::Mat *inmask)
		:x(inx),y(iny),mask(inmask)
	{
		this->width = inmask->cols;
		this->height = inmask->rows;
	}

	BodyInfoClass::~BodyInfoClass()
	{
	}


}




