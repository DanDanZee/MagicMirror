#pragma once
#ifndef _BODYDETECTTASKCLASS_H_
#define _BODYDETECTTASKCLASS_H_

#include <vector>
#include <string>
#include "Kinect.h"
#include "Kinect.Face.h"
#include "Kinect.VisualGestureBuilder.h"
#include "TaskClass.h"
#include "MsgQueClass.h"
#include "BodyInfoClass.h"
#include "PersonClass.h"

#define  AGE_CATEGORY				4
#define  STYLE_AGE_CATEGORY			5
#define  FACE_CROP_SIZE				600
#define  DETECT_THREAD_SLEEP		60ms
#define  DETECT_THREAD_MIRROR_SLEEP 40ms
#define	 MEAN_STD_OUTPUT_DIM1		512
#define	 MEAN_STD_OUTPUT_DIM2		1
#define	 MEAN_STD_OUTPUT_DIM3		1

namespace InTheGlassRoom {
	class BodyDetectTaskClass :public TaskClass {
	public:
		BodyDetectTaskClass(const std::string name);
		~BodyDetectTaskClass();
		void			run();
		bool			initSensor();
		void			stop();
		void			setMsgQueue(MsgQueClass<std::vector<tensorflow::Tensor *>> *que);
		void			setMirrorMsgQueue(MsgQueClass<cv::Mat> *que);
		void			setControlMsgQueue(MsgQueClass<int> *que);
		void			setbodyinfoMsgQueue(MsgQueClass<std::vector<BodyInfoClass>> *que);
		void			beforeChangeStatus();
	private:

		bool			initTaskClass();
		void			mainLoop();
		int				SamplingInterval=100;

		MsgQueClass<std::vector<tensorflow::Tensor *>>	*msgQue;

		MsgQueClass<cv::Mat>			*mirrorMsgQue;

		MsgQueClass<int>				*controlMsgQue;

		MsgQueClass<std::vector<BodyInfoClass>>		*bodyinfoMsgQue;
		//pointer of kinect sensor
		IKinectSensor*			pSensor				= nullptr;
		//pointer of color image reader
		IColorFrameReader*		pColorFrameReader	= nullptr;
		//depth image reader
		IDepthFrameReader*		pDepthFrameReader	= nullptr;
		//bodies index reader
		IBodyIndexFrameReader*	pBIFrameReader		= nullptr;
		//depth --> color image mapper
		ICoordinateMapper*		pCoordinateMapper	= nullptr;

		IBodyFrameReader*		pBodyFrameReader	= nullptr;

		UINT uColorBufferSize	= 0;
		
		UINT uColorPointNum		= 0;
		
		int iColorWidth			= 0;

		int iColorHeight		= 0;
		// total point number of depth image
		UINT uDepthPointNum		= 0;
		// width of depth image
		int iDepthWidth			= 0;
		// height of depth image
		int iDepthHeight		= 0;

		INT32 iBodyCount		= 0;

		IBody**					aBody;
		//face readers
		IFaceFrameReader*		facereader[BODY_COUNT];
		//face source
		IFaceFrameSource*		facesource[BODY_COUNT];
		//face detect configure
		DWORD					features;
		// gesture count
		UINT					iGestureCount		= 0;

		IGesture**				aGestureList;
		IVisualGestureBuilderFrameSource** aGestureSources;
		IVisualGestureBuilderFrameReader** aGestureReaders;

		InTheGlassRoom::PersonClass				person[BODY_COUNT];

		std::unique_ptr<tensorflow::Session>	style_mean_std_session;

		tensorflow::Tensor						style_means[AGE_CATEGORY];
		tensorflow::Tensor						style_stds[AGE_CATEGORY];

	};
}

#endif // !_BODYDETECTTASKCLASS_H_
