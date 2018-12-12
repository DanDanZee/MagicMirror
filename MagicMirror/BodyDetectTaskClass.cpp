#define COMPILER_MSVC
#define NOMINMAX
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>
#include "Kinect.h"
#include "Kinect.VisualGestureBuilder.h"
#include "Kinect.Face.h"
#include "tensorflow/core/platform/logging.h"
#include "BodyDetectTaskClass.h"
#include "Tools.h"
#include "BodyInfoClass.h"
#include "classifier.h"
#include "facealign.h"
#include "PersonClass.h"

namespace InTheGlassRoom {
	BodyDetectTaskClass::BodyDetectTaskClass(const string name)
		:TaskClass(name)
	{

	}

	BodyDetectTaskClass::~BodyDetectTaskClass() {

	}

	void BodyDetectTaskClass::stop() {
		this->canRun = false;
		// release coordinate mapper
		LOG(INFO) << "Release coordinate mapper";
		pCoordinateMapper->Release();
		pCoordinateMapper = nullptr;

		// release frame reader
		LOG(INFO) << "Release frame reader";
		pColorFrameReader->Release();
		pColorFrameReader = nullptr;
		pDepthFrameReader->Release();
		pDepthFrameReader = nullptr;
		pBIFrameReader->Release();
		pBIFrameReader = nullptr;
		pBodyFrameReader->Release();
		pBodyFrameReader = nullptr;
		LOG(INFO) << "Release face reader and source";
		for (int i = 0; i < BODY_COUNT; i++) {
			facereader[i]->Release();
			facereader[i] = nullptr;
			facesource[i]->Release();
			facesource[i] = nullptr;
		}
		LOG(INFO) << "Release gesture reader and source";
		for (int i = 0; i < iBodyCount; ++i)
		{
			aGestureReaders[i]->Release();
			aGestureSources[i]->Release();
		}
		delete[] aGestureReaders;
		delete[] aGestureSources;

		// Close Sensor
		LOG(INFO) << "close sensor";
		pSensor->Close();

		// Release Sensor
		LOG(INFO) << "Release sensor";
		pSensor->Release();
		pSensor = nullptr;
	}

	bool BodyDetectTaskClass::initTaskClass() {
		LOG(INFO) << "Start to init the kinect sensor";
		LOG(INFO) << "Try to get default sensor";
		{
			if (GetDefaultKinectSensor(&pSensor) != S_OK)
			{
				LOG(ERROR) << "Get Sensor failed";
				return false;
			}
			LOG(INFO) << "Try to open sensor";
			if (pSensor->Open() != S_OK)
			{
				LOG(ERROR) << "Can't open sensor";
				return false;
			}
		}
		LOG(INFO) << "Try to get color source";
		{
			// Get frame source
			IColorFrameSource* pFrameSource = nullptr;
			if (pSensor->get_ColorFrameSource(&pFrameSource) != S_OK)
			{
				LOG(ERROR) << "Can't get color frame source";
				return false;
			}
			// Get frame description
			LOG(INFO) << "get color frame description";
			IFrameDescription* pFrameDescription = nullptr;
			if (pFrameSource->get_FrameDescription(&pFrameDescription) == S_OK)
			{
				pFrameDescription->get_Width(&iColorWidth);
				pFrameDescription->get_Height(&iColorHeight);

				uColorPointNum = iColorWidth * iColorHeight;
				uColorBufferSize = uColorPointNum * 4 * sizeof(BYTE);
			}
			pFrameDescription->Release();
			pFrameDescription = nullptr;

			// get frame reader
			LOG(INFO) << "Try to get color frame reader";
			if (pFrameSource->OpenReader(&pColorFrameReader) != S_OK)
			{
				LOG(ERROR) << "Can't get color frame reader";
				return false;
			}
			// release Frame source
			LOG(INFO) << "Release frame source";
			pFrameSource->Release();
			pFrameSource = nullptr;
		}
		LOG(INFO) << "Try to get depth source";
		{
			// Get frame source
			IDepthFrameSource* pFrameSource = nullptr;
			if (pSensor->get_DepthFrameSource(&pFrameSource) != S_OK)
			{
				LOG(ERROR) << "Can't get depth frame source";
				return false;
			}
			// Get frame description
			LOG(INFO) << "get depth frame description";
			IFrameDescription* pFrameDescription = nullptr;
			if (pFrameSource->get_FrameDescription(&pFrameDescription) == S_OK)
			{
				pFrameDescription->get_Width(&iDepthWidth);
				pFrameDescription->get_Height(&iDepthHeight);
				uDepthPointNum = iDepthWidth * iDepthHeight;
			}
			pFrameDescription->Release();
			pFrameDescription = nullptr;
			// get frame reader
			LOG(INFO) << "Try to get depth frame reader";
			if (pFrameSource->OpenReader(&pDepthFrameReader) != S_OK)
			{
				LOG(ERROR) << "Can't get depth frame reader";
				return false;
			}
			// release Frame source
			LOG(INFO) << "Release frame source";
			pFrameSource->Release();
			pFrameSource = nullptr;
		}
		LOG(INFO) << "Try to get body index source";
		{
			// Get frame source
			IBodyIndexFrameSource* pFrameSource = nullptr;
			if (pSensor->get_BodyIndexFrameSource(&pFrameSource) != S_OK)
			{
				LOG(ERROR) << "Can't get body index frame source";
				return false;
			}
			// get frame reader
			LOG(INFO) << "Try to get body index frame reader";
			if (pFrameSource->OpenReader(&pBIFrameReader) != S_OK)
			{
				LOG(ERROR) << "Can't get depth frame reader";
				return false;
			}
			// release Frame source
			LOG(INFO) << "Release frame source";
			pFrameSource->Release();
			pFrameSource = nullptr;
		}
		LOG(INFO) << "Try to get Coordinate Mapper";
		if (pSensor->get_CoordinateMapper(&pCoordinateMapper) != S_OK)
		{
			LOG(ERROR) << "get_CoordinateMapper failed";
			return false;
		}
		LOG(INFO) << "Try to get body source";
		{
			IBodyFrameSource* pBodyFrameSource = nullptr;
			if (pSensor->get_BodyFrameSource(&pBodyFrameSource) != S_OK)
			{
				LOG(ERROR) << "Can't get body frame source";
				return false;
			}
			if (pBodyFrameSource->get_BodyCount(&iBodyCount) != S_OK)
			{
				LOG(ERROR) << "Can't get body count";
				return false;
			}
			LOG(INFO) << "Try to get body frame reader";
			if (pBodyFrameSource->OpenReader(&pBodyFrameReader) != S_OK)
			{
				LOG(ERROR) << "Can't get body frame reader";
				return false;
			}
			LOG(INFO) << "Release body frame source";
			pBodyFrameSource->Release();
			pBodyFrameSource = nullptr;
			LOG(INFO) << " > Can trace " << iBodyCount << " bodies";
		}
		// Allocate resource for bodies
		aBody = new IBody*[iBodyCount];
		for (int i = 0; i < iBodyCount; ++i)
			aBody[i] = nullptr;
		features = FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
			| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
			| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
			| FaceFrameFeatures::FaceFrameFeatures_Happy
			| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
			| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
			| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
			| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
			| FaceFrameFeatures::FaceFrameFeatures_LookingAway
			| FaceFrameFeatures::FaceFrameFeatures_Glasses
			| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;
		HRESULT hResult;
		LOG(INFO) << "Try to get face frame reader";
		{
			for (int i = 0; i < BODY_COUNT; i++)
			{
				hResult = CreateFaceFrameSource(pSensor, 0, features, &facesource[i]);
				if (FAILED(hResult))
				{
					LOG(ERROR) << "Error : CreateFaceFrameSource";
					return false;
				}
				facesource[i]->OpenReader(&facereader[i]);
			}
		}
		// Load gesture dataase from File
		wstring sDatabaseFile = L"SwagHandProgress.gba";	// Modify this file to load other file
		IVisualGestureBuilderDatabase* pGestureDatabase = nullptr;
		LOG(INFO) << "Try to load gesture database file";
		if (CreateVisualGestureBuilderDatabaseInstanceFromFile(sDatabaseFile.c_str(), &pGestureDatabase) != S_OK)
		{
			LOG(ERROR) << "Can't read database file ";
			return false;
		}
		// Get the number of gestures in database

		LOG(INFO) << "Try to read gesture list";
		if (pGestureDatabase->get_AvailableGesturesCount(&iGestureCount) != S_OK)
		{
			LOG(ERROR) << "Can't read the gesture count";
			return false;
		}
		if (iGestureCount == 0)
		{
			LOG(ERROR) << "There is no gesture in the database";
			return false;
		}

		// get the list of gestures
		aGestureList = new IGesture*[iGestureCount];
		if (pGestureDatabase->get_AvailableGestures(iGestureCount, aGestureList) != S_OK)
		{
			LOG(ERROR) << "Can't read the gesture list";
			return false;
		}
		else
		{
			// output the gesture list
			LOG(INFO) << "There are " << iGestureCount << " gestures in the database: ";
			GestureType mType;
			const UINT uTextLength = 260; // magic number, if value smaller than 260, can't get name
			wchar_t sName[uTextLength];
			for (UINT i = 0; i < iGestureCount; ++i)
			{
				if (aGestureList[i]->get_GestureType(&mType) == S_OK)
				{
					if (mType == GestureType_Discrete)
						LOG(INFO) << "Gesture Type: Discrete";
					else if (mType == GestureType_Continuous)
						LOG(INFO) << "Gesture Type: Continuous";
					if (aGestureList[i]->get_Name(uTextLength, sName) == S_OK)
						LOG(INFO) << sName;
				}
			}
		}
		aGestureSources = new IVisualGestureBuilderFrameSource*[iBodyCount];
		aGestureReaders = new IVisualGestureBuilderFrameReader*[iBodyCount];
		LOG(INFO) << "Try to create IVisualGestureBuilderFrameSource";
		{
			for (int i = 0; i < iBodyCount; ++i)
			{
				// frame source
				aGestureSources[i] = nullptr;
				if (CreateVisualGestureBuilderFrameSource(pSensor, 0, &aGestureSources[i]) != S_OK)
				{
					LOG(ERROR) << "Can't create IVisualGestureBuilderFrameSource";
					return false;
				}

				// set gestures
				if (aGestureSources[i]->AddGestures(iGestureCount, aGestureList) != S_OK)
				{
					LOG(ERROR) << "Add gestures failed";
					return false;
				}

				// frame reader
				aGestureReaders[i] = nullptr;
				if (aGestureSources[i]->OpenReader(&aGestureReaders[i]) != S_OK)
				{
					LOG(ERROR) << "Can't open IVisualGestureBuilderFrameReader";
					return false;
				}
			}
		}

		LOG(INFO) << "Try to load styles";
		std::string style_input_name = "style_input:0";
		std::string output_layer_name = "transpose_1:0";
		std::string input_style_mean_name = "style_mean_input:0";
		std::string input_style_std_name = "style_std_input:0";
		std::string style_input_mean_name = "moments/mean:0";
		std::string style_input_std_name = "Sqrt:0";
		std::string	graph_path2 = "D:/fronzen_model/style_mean_std_frozen_modified.pd";
		std::string age[AGE_CATEGORY][STYLE_AGE_CATEGORY] = { { "D:\\style\\age1\\1.jpg","D:\\style\\age1\\2.jpg","D:\\style\\age1\\3.jpg","D:\\style\\age1\\4.jpg","D:\\style\\age1\\5.jpg" },
		{ "D:\\style\\age2\\1.jpg","D:\\style\\age2\\2.jpg","D:\\style\\age2\\3.jpg","D:\\style\\age2\\4.jpg","D:\\style\\age2\\5.jpg" },
		{ "D:\\style\\age3\\1.jpg","D:\\style\\age3\\2.jpg","D:\\style\\age3\\3.jpg","D:\\style\\age3\\4.jpg","D:\\style\\age3\\5.jpg" },
		{ "D:\\style\\age4\\1.jpg","D:\\style\\age4\\2.jpg","D:\\style\\age4\\3.jpg","D:\\style\\age4\\4.jpg","D:\\style\\age4\\5.jpg" }
		};
		std::vector<tensorflow::Tensor>			mean_std_outputs;
		tensorflow::Tensor						*style_tensors;
		tensorflow::Status						load_graph_status = LoadGraph(graph_path2, &style_mean_std_session);
		if (!load_graph_status.ok()) {
			LOG(ERROR) << load_graph_status;
			return false;
		}
		else {
			LOG(INFO) << "Style mean and std Graph load success!";
		}
		for (int i = 0; i < AGE_CATEGORY; i++) {
			//get the mean and std of style img
			style_tensors = ReadStyleImage2Tensor(age[i], STYLE_AGE_CATEGORY);
				tensorflow::Status style_mean_std_run_status
				= style_mean_std_session->Run({ { style_input_name ,*style_tensors } }, { style_input_mean_name,style_input_std_name }, {}, &mean_std_outputs);
			if (!style_mean_std_run_status.ok()) {
				LOG(ERROR) << "Running model failed: " << style_mean_std_run_status;
				return false;
			}
			style_means[i] = mean_std_outputs[0];
			style_stds[i] = mean_std_outputs[1];
		}
		LOG(INFO) << "Get mean and std of the style image";
		for (int i = 0; i < BODY_COUNT; i++) {
			person[i] = PersonClass();
		}
		return true;
	}

	bool BodyDetectTaskClass::initSensor() {
		if (this->initTaskClass()) {
			LOG(INFO) << "Sensor OK!";
			return true;
		}
		else {
			LOG(ERROR) << "Sensor init failed!";
			return false;
		}
	}

	void BodyDetectTaskClass::setMsgQueue(MsgQueClass<std::vector<tensorflow::Tensor *>> *que) {
		this->msgQue = que;
	}

	void BodyDetectTaskClass::setMirrorMsgQueue(MsgQueClass<cv::Mat> *que) {
		this->mirrorMsgQue = que;
	}

	void BodyDetectTaskClass::setControlMsgQueue(MsgQueClass<int> *que) {
		this->controlMsgQue = que;
	}

	void BodyDetectTaskClass::setbodyinfoMsgQueue(MsgQueClass<std::vector<BodyInfoClass>> *que) {
		this->bodyinfoMsgQue = que;
	}

	void BodyDetectTaskClass::mainLoop() {
		//kinect color image
		cv::Mat	imgColor(iColorHeight, iColorWidth, CV_8UC4);
		cv::Mat imgTarget(iColorHeight, iColorWidth, CV_8UC3);
		//mask for six bodies
		cv::Mat Mask[BODY_COUNT];
		for (int i = 0; i < BODY_COUNT; i++) {
			Mask[i] = cv::Mat(iColorHeight, iColorWidth, CV_8UC1);
		}
		bool				trackbody[BODY_COUNT] = { false };
		UINT16*				pDepthPoints = new UINT16[uDepthPointNum];
		BYTE*				pBodyIndex = new BYTE[uDepthPointNum];
		DepthSpacePoint*	pPointArray = new DepthSpacePoint[uColorPointNum];
		unsigned int		frame_num = 0;
		//Hand state 
		char Lasso_detected_count[BODY_COUNT] = { 0 };
		char Closed_detected_count[BODY_COUNT] = { 0 };
		char Open_detected_count[BODY_COUNT] = { 0 };
		char Hand_State_detect_num = 0;
		bool open_hand = false, close_hand = false, lasso_hand = false;
		//last time the hand state
		char Last_Hand_State[BODY_COUNT] = { 3,3,3,3,3,3 };

		//age and face
		Classifier classfier = Classifier();
		FaceAlign  p_Align = FaceAlign();
		//operation result
		HRESULT hResult;
		float scaleX = 1.6, scaledX = 0.0, scaleY = 1.8, scaledY = 0.0;
		int scaled_width;
		int scaled_height;
		int leftTopX, leftTopY, rightBottomX, rightBottomY;
		scaledX = (scaleX *0.5 - 0.5);
		scaledY = (scaleY *0.5 - 0.5);
		cv::Mat *faces[BODY_COUNT];
		cv::Mat tempmat;
		bool happy[BODY_COUNT] = { false };
		DetectionResult faceProperties[FaceProperty::FaceProperty_Count];

		int upline[BODY_COUNT] = { 0 };
		int downline[BODY_COUNT] = { 0 };
		bool isup[BODY_COUNT] = { false };
		bool move_up_hand = false;
		//main loop
		while (this->canRun) {
			DWORD k = ::GetTickCount();
			// 8a. Read color frame
			IColorFrame* pColorFrame = nullptr;
			if (pColorFrameReader->AcquireLatestFrame(&pColorFrame) == S_OK)
			{
				pColorFrame->CopyConvertedFrameDataToArray(uColorBufferSize, imgColor.data, ColorImageFormat_Bgra);
				pColorFrame->Release();
				pColorFrame = nullptr;
			}
			// detect the Hand State
			IBodyFrame* pBodyFrame = nullptr;
			if (pBodyFrameReader->AcquireLatestFrame(&pBodyFrame) == S_OK)
			{
				// 4b. get Body data
				if (pBodyFrame->GetAndRefreshBodyData(iBodyCount, aBody) == S_OK)
				{
					cv::Mat imgBG(iColorHeight, iColorWidth, CV_8UC3);
					BOOLEAN detected_gesture = false;
					// 4c. for each body
					for (int i = 0; i < iBodyCount; ++i)
					{
						IBody* pBody = aBody[i];
						// check if is tracked
						BOOLEAN bTracked = false;
						if ((pBody->get_IsTracked(&bTracked) == S_OK) && bTracked)
						{
							UINT64 trackingId = _UI64_MAX;
							hResult = pBody->get_TrackingId(&trackingId);
							if (SUCCEEDED(hResult)) {
								facesource[i]->put_TrackingId(trackingId);
							}
							// get tracking ID of body
							//HandState leftHandState = HandState_Unknown;
							HandState rightHandState = HandState_Unknown;
							//pBody->get_HandLeftState(&leftHandState);
							pBody->get_HandRightState(&rightHandState);
							if (rightHandState == HandState_Lasso)
								Lasso_detected_count[i]++;
							if (rightHandState == HandState_Closed)
								Closed_detected_count[i]++;
							if (rightHandState == HandState_Open)
								Open_detected_count[i]++;
							if (Hand_State_detect_num == 4) {
								float temp = (float)Lasso_detected_count[i] / 5.0;
								if (temp >= 0.8) {
									if (Last_Hand_State[i] != 0) {
										lasso_hand = true;
									}
									Last_Hand_State[i] = 0;
								}
								temp = (float)Closed_detected_count[i] / 5.0;
								if (temp >= 0.8) {
									if (Last_Hand_State[i] == 2) {
										close_hand = true;
									}
									Last_Hand_State[i] = 1;
								}
								temp = (float)Open_detected_count[i] / 5.0;
								if (temp >= 0.8) {
									if (Last_Hand_State[i] == 1) {
										open_hand = true;
									}
									Last_Hand_State[i] = 2;
								}
							}
							UINT64 uTrackingId = 0;
							if (pBody->get_TrackingId(&uTrackingId) == S_OK)
							{
								// get tracking id of gesture
								UINT64 uGestureId = 0;
								if (aGestureSources[i]->get_TrackingId(&uGestureId) == S_OK)
								{
									if (uGestureId != uTrackingId)
									{
										// assign traking ID if the value is changed
										LOG(INFO) << "Gesture Source " << i << " start to track user " << uTrackingId;
										aGestureSources[i]->put_TrackingId(uTrackingId);
									}
								}
							}
							IVisualGestureBuilderFrame* pGestureFrame = nullptr;
							if (aGestureReaders[i]->CalculateAndAcquireLatestFrame(&pGestureFrame) == S_OK)
							{
								// check if the gesture of this body is tracked
								BOOLEAN bGestureTracked = false;
								if (pGestureFrame->get_IsTrackingIdValid(&bGestureTracked) == S_OK && bGestureTracked)
								{
									// for each gestures
									for (UINT j = 0; j < iGestureCount; ++j)
									{
										// get gesture information
										//aGestureList[j]->get_Name(uTextLength, sName);
										// get gesture result
										IContinuousGestureResult* pGestureResult = nullptr;
										if (pGestureFrame->get_ContinuousGestureResult(aGestureList[j], &pGestureResult) == S_OK)
										{
											// get progress
											float fProgress = 0.0f;
											if (pGestureResult->get_Progress(&fProgress) == S_OK)
											{
												if (fProgress > 0.6) {
													upline[i]++;
													downline[i] = 0;
													if (upline[i] > 5) {
														isup[i] = true;
														upline[i] = 0;
													}
												}
												else {
													downline[i]++;
													upline[i] = 0;
													if (downline[i] > 5) {
														if (isup[i]) {
															move_up_hand = true;
														}
														isup[i] = false;
														downline[i] = 0;
													}
												}
											}
											pGestureResult->Release();
										}
									}
								}
								pGestureFrame->Release();
							}
						}//for each body code end
					}
					Hand_State_detect_num++;
					if (move_up_hand) {
						move_up_hand = false;
						/*int * cmd = new int();
						*cmd = 4;
						this->controlMsgQue->sendMsg(102, cmd);*/
						LOG(INFO) << "detect hand up move";
					}
					if (Hand_State_detect_num == 5) {
						Hand_State_detect_num = 0;
						for (int i = 0; i < BODY_COUNT; i++) {
							Lasso_detected_count[i] = 0;
							Closed_detected_count[i] = 0;
							Open_detected_count[i] = 0;
						}
						//only performence in status =1
						if (lasso_hand) {
							lasso_hand = false;
							if (status == 1) {
								int * cmd = new int();
								*cmd = 1;
								this->controlMsgQue->sendMsg(103, cmd);
							}
							//person[i].ChangeStyle();
							LOG(INFO) << "detect hand lasso";
						}
						if (open_hand) {
							open_hand = false;
							if (status == 0) {
								int * cmd = new int();
								*cmd = 3;
								this->controlMsgQue->sendMsg(104, cmd);
							}
							LOG(INFO) << "detect hand Openning";
						}
						//only performence status =1
						if (close_hand) {
							close_hand = false;
							if (status == 1) {
								int * cmd = new int();
								*cmd = 2;
								this->controlMsgQue->sendMsg(105, cmd);
							}
							LOG(INFO) << "detect hand Closing";
						}
					}

				}
				pBodyFrame->Release();
				pBodyFrame = nullptr;
			}
			for (int i = 0; i < BODY_COUNT; i++)
			{
				IFaceFrame *faceframe;
				hResult = facereader[i]->AcquireLatestFrame(&faceframe);
				if (faceframe == nullptr)
					continue;
				if (SUCCEEDED(hResult) && faceframe != nullptr)
				{
					BOOLEAN tracked = false;
					hResult = faceframe->get_IsTrackingIdValid(&tracked);
					if (SUCCEEDED(hResult) && tracked)
					{
						LOG(WARNING) << i << "-th tracking status: " << tracked;
						IFaceFrameResult *faceresult = nullptr;
						hResult = faceframe->get_FaceFrameResult(&faceresult);
						if (SUCCEEDED(hResult))
						{
							hResult = faceresult->GetFaceProperties(FaceProperty::FaceProperty_Count, faceProperties);
							if (SUCCEEDED(hResult)) {
								//detect laugh
								if (faceProperties[0] == DetectionResult::DetectionResult_Yes) {
									if (!happy[i]) {
										LOG(INFO) << i << "th person turn to laughing";
										if(this->status==1)
										person[i].ChangeStyle();
									}
									happy[i] = true;
								}
								else {
									happy[i] = false;
								}
							}
							LOG(INFO) << i << "th is detected: " << trackbody[i];
							if (trackbody[i] == true) {
								continue;
							}
							trackbody[i] = true;
							RectI box;
							hResult = faceresult->get_FaceBoundingBoxInColorSpace(&box);
							if (SUCCEEDED(hResult))
							{
								scaled_width = box.Right - box.Left;
								scaled_height = box.Bottom - box.Top;
								if (scaled_width > 0 && scaled_height > 0)
								{
									leftTopX = box.Left - scaled_width*scaledX;
									leftTopY = box.Top - scaled_height*scaledY;
									if (leftTopX <= 0)
										leftTopX = 0;
									if (leftTopY <= 0)
										leftTopY = 0;
									std::vector<cv::Point2d> landmarks;
									landmarks.clear();
									PointF facepoint[FacePointType_Count];
									hResult = faceresult->GetFacePointsInColorSpace(FacePointType_Count, facepoint);
									if (SUCCEEDED(hResult))
									{
										landmarks.push_back(cv::Point2d(facepoint[0].X - leftTopX, facepoint[0].Y - leftTopY));
										landmarks.push_back(cv::Point2d(facepoint[1].X - leftTopX, facepoint[1].Y - leftTopY));
										landmarks.push_back(cv::Point2d(facepoint[2].X - leftTopX, facepoint[2].Y - leftTopY));
										landmarks.push_back(cv::Point2d(facepoint[3].X - leftTopX, facepoint[3].Y - leftTopY));
										landmarks.push_back(cv::Point2d(facepoint[4].X - leftTopX, facepoint[4].Y - leftTopY));
									}
									cv::Rect faceregion(leftTopX, leftTopY, scaled_width*scaleX, scaled_height*scaleY);
									Mat roi = imgColor(faceregion);
									faces[i] = new cv::Mat(roi.rows, roi.cols, CV_8UC3);
									for (int y = 0; y < roi.rows; ++y)
									{
										uchar* colorData = roi.ptr(y);
										uchar* targetData = faces[i]->ptr(y);
										for (int x = 0; x < roi.cols; ++x)
										{
											targetData[x * 3] = colorData[x * 4];
											targetData[x * 3 + 1] = colorData[x * 4 + 1];
											targetData[x * 3 + 2] = colorData[x * 4 + 2];
										}
									}
									cv::Mat aligned_face = p_Align.align_OneFace(*faces[i], landmarks);
									int age = classfier.getAge_fromAligned(aligned_face, 117);
									// 0~19
									if (age < 20) {
										person[i].setStyle(style_means[0], style_stds[0]);
									}
									// 20~25
									else if (age > 19 && age < 26) {
										person[i].setStyle(style_means[1], style_stds[1]);
									}
									// 26~29
									else if (age > 25 && age < 30) {
										person[i].setStyle(style_means[2], style_stds[2]);
									}
									// >=30
									else {
										person[i].setStyle(style_means[3], style_stds[3]);
									}
									cv::Point origin;
									origin.x = 25;
									origin.y = 25;
									int height = faces[i]->cols >> 1;
									int width = faces[i]->rows >> 1;
									cv::resize(*faces[i], *faces[i], cv::Size(width, height));
									cv::putText(*faces[i], to_string(age), origin, 1.8, 1.8, cv::Scalar(0, 0, 255), 1, 8, 0);
								}
							}
						}
						faceresult->Release();
						faceresult = nullptr;
					}
					else {
						trackbody[i] = false;
						faces[i] = nullptr;
					}
				}
				else {
					LOG(ERROR) << "face frame error!!";
				}
			}
			//mirror mode
			if (this->status == 0) {
				cv::Mat *tempmat = new cv::Mat(iColorHeight, iColorWidth, CV_8UC3);
				for (int y = 0; y < imgColor.rows; ++y)
				{
					uchar* colorData = imgColor.ptr(y);
					uchar* targetData = tempmat->ptr(y);
					for (int x = 0; x < imgColor.cols; ++x)
					{
						targetData[x * 3]		= colorData[x * 4];
						targetData[x * 3 + 1]	= colorData[x * 4 + 1];
						targetData[x * 3 + 2]	= colorData[x * 4 + 2];
					}
				}
				int yoffset = 0;
				for (int i = 0; i < BODY_COUNT; i++) {
					if (faces[i] == nullptr)
						continue;
					cv::Mat	 temp = *faces[i];
					cv::Rect temprect = cv::Rect(0, yoffset, temp.cols, temp.rows);
					yoffset += temp.rows;
					temp.copyTo((*tempmat)(temprect));
				}
				this->mirrorMsgQue->sendMsg(frame_num++, tempmat);
				std::this_thread::sleep_for(DETECT_THREAD_MIRROR_SLEEP);
				continue;
			}

			// 8b. read depth frame
			IDepthFrame* pDepthFrame = nullptr;
			if (pDepthFrameReader->AcquireLatestFrame(&pDepthFrame) == S_OK)
			{
				pDepthFrame->CopyFrameDataToArray(uDepthPointNum, pDepthPoints);
				pDepthFrame->Release();
				pDepthFrame = nullptr;
			}

			// 8c. read body index frame
			IBodyIndexFrame* pBIFrame = nullptr;
			if (pBIFrameReader->AcquireLatestFrame(&pBIFrame) == S_OK)
			{
				pBIFrame->CopyFrameDataToArray(uDepthPointNum, pBodyIndex);
				pBIFrame->Release();
				pBIFrame = nullptr;
			}
			//set mask to zeros
			for (int j = 0; j < 6; j++) {
				Mask[j].setTo(0);
			}
			// 9b. map color to depth
			if (pCoordinateMapper->MapColorFrameToDepthSpace(uDepthPointNum, pDepthPoints, uColorPointNum, pPointArray) == S_OK)
			{
				for (int y = 0; y < imgColor.rows; ++y)
				{
					uchar* colorData = imgColor.ptr(y);
					uchar* targetData = imgTarget.ptr(y);
					for (int x = 0; x < imgColor.cols; ++x)
					{
						targetData[x * 3]		= colorData[x * 4];
						targetData[x * 3 + 1]	= colorData[x * 4 + 1];
						targetData[x * 3 + 2]	= colorData[x * 4 + 2];
						// ( x, y ) in color frame = rPoint in depth frame
						const DepthSpacePoint& rPoint = pPointArray[y * imgColor.cols + x];
						// check if rPoint is in range
						if (rPoint.X >= 0 && rPoint.X < iDepthWidth && rPoint.Y >= 0 && rPoint.Y < iDepthHeight)
						{
							// fill color from color frame if this pixel is user
							int iIdx = (int)rPoint.X + iDepthWidth * (int)rPoint.Y;
							if (pBodyIndex[iIdx] < BODY_COUNT)
							{
								Mask[pBodyIndex[iIdx]].ptr<uchar>(y)[x] = 255;
							}
						}
					}
				}
			}
			std::vector<cv::Mat> *mats = new std::vector<cv::Mat>();
			std::vector<BodyInfoClass> *infos = new std::vector<BodyInfoClass>();
			for (int t = 0; t < BODY_COUNT; t++) {
				if (trackbody[t]) {
					if (faces[t] == nullptr) {
						trackbody[t] = false;
						continue;
					}
					cv::morphologyEx(Mask[t], Mask[t], cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11)));
					vector <vector<cv::Point>> contours;
					cv::findContours(Mask[t], contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
					cv::Rect boundRect;
					for (int i = 0; i < contours.size(); i++)
					{
						if (contours[i].size() < 2000)
							continue;
						boundRect = cv::boundingRect(cv::Mat(contours[i]));
					}
					if (boundRect.height <= 0 || boundRect.width <= 0) {
						LOG(WARNING) << "Found a boundRect height="<< boundRect.height<<" width="<< boundRect.width;
						continue;
					}
						
					cv::Mat temp(boundRect.width,boundRect.height, CV_8UC3);
					cv::Mat* resultMask = new cv::Mat();
					*resultMask = Mask[t](boundRect).clone();
					imgTarget(boundRect).copyTo(temp);
					cv::resize(temp, temp, cv::Size(FACE_CROP_SIZE, FACE_CROP_SIZE));
					BodyInfoClass bodyinfo = BodyInfoClass(boundRect.x, boundRect.y, resultMask);
					//此处有可能有bug
					cv::Mat *tempface = new cv::Mat();
					LOG(INFO) << t << "-th face " << faces[t];
					*tempface = faces[t]->clone();
					bodyinfo.face = tempface;
					mats->push_back(temp);
					infos->push_back(bodyinfo);
					
				}
			}
			if (mats->size() > 0&&mats->size()<7) {
				//body images
				tensorflow::Tensor * temptensor = Mat2Tensor_HWC(mats);
				//styles
				tensorflow::Tensor * stylemeantensor
					= new tensorflow::Tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ (int)mats->size(), MEAN_STD_OUTPUT_DIM1, MEAN_STD_OUTPUT_DIM2, MEAN_STD_OUTPUT_DIM3 }));
				tensorflow::Tensor * stylestdtensor
					= new tensorflow::Tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ (int)mats->size(), MEAN_STD_OUTPUT_DIM1, MEAN_STD_OUTPUT_DIM2, MEAN_STD_OUTPUT_DIM3 }));

				for (int t = 0,w=0; t < BODY_COUNT; t++) {
					if (trackbody[t]) {
						//insert style mean and std tensor
						person[t].getStyle(stylemeantensor->Slice(w, w + 1), stylestdtensor->Slice(w, w + 1));
						w++;
					}
				}
				std::vector<tensorflow::Tensor *> *styletensors = new std::vector<tensorflow::Tensor *>();
				styletensors->push_back(temptensor);
				styletensors->push_back(stylemeantensor);
				styletensors->push_back(stylestdtensor);
				this->msgQue->sendMsg(frame_num, styletensors);
				this->bodyinfoMsgQue->sendMsg(frame_num, infos);
				frame_num++;
			}
			delete mats;
			LOG(INFO) << " detect totally time consum " << ::GetTickCount() - k;
			std::this_thread::sleep_for(DETECT_THREAD_SLEEP);
		}
		delete[] pPointArray;
		delete[] pBodyIndex;
		delete[] pDepthPoints;
		LOG(INFO) << "body detect thread stop";
	}
	void BodyDetectTaskClass::run() {
		this->canRun = true;
		LOG(INFO) << "ready to run detect thread";
		//申明一个线程并运行它
		this->taskThread = new thread(&BodyDetectTaskClass::mainLoop, this);
		//return true;
	}
	void BodyDetectTaskClass::beforeChangeStatus() {

	}
}