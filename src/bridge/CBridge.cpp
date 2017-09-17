//==============================================================================
/*

\author    Hojun Cha
\version   0.0.1
*/
//==============================================================================

#define TEST 1

#include "bridge/CBridge.h"

using namespace std;
namespace chai3d {
	
/* Constructor */
cBridge::cBridge() {
	currentViewNumber = 0;
	numberOfView = 0;
	sizeOfView = new int[0]{};
	
	infoMsg = "";
	errorMsg = "";

	//if (updateFileList()) {
	//	errorMsg = "Update file list is not working";
	//}
}

/* Grouping functions */
bool cBridge::Tick() {
	bool isSuccess = true;
	
	if (!uploadHIPData()) {
		isSuccess = false;
		errorMsg = "Fail to upload HIP Info.";
	}

	if (!updateObject()) {
		isSuccess = false;
		errorMsg = "Failed to update object Info.";
	}

	return true;
}

bool cBridge::registerObject(int objectNumber, cMultiMesh* object) {
	objectMap[objectNumber] = object;

	return true;
}

//bool cBridge::registerObjects(int objectNumber, vector<cMultiMesh *> obj) {
//	objects = obj;
//
//	return true;
//}
//
//vector<cMultiMesh*> cBridge::getObjects() {
//	return objects;
//}

bool cBridge::updateFileList() {
	return registerFileInFolder("../../../../Mesh");
}

bool cBridge::registerFileInFolder(string path) {
	WIN32_FIND_DATA fi;
	
	string pathToSearch = path;
	if (pathToSearch.size() != 0)
		pathToSearch = path + "/";

	HANDLE h = FindFirstFile((pathToSearch + "*").c_str(), &fi);

	if (h != INVALID_HANDLE_VALUE) {
		do {
			string fileName = fi.cFileName;

			if (fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if ((fileName != ".") && (fileName != "..")) {
					registerFileInFolder(pathToSearch + fileName);
				}
			} else {
				if (filePaths.find(fileName) == filePaths.end()) // does not exist then register
					filePaths[fileName] = pathToSearch + fileName;
			}
		} while (FindNextFile(h, &fi));
	}
	
	return true;
}

/* Registering function */
bool cBridge::registerHapticDevice(cGenericHapticDevicePtr device) {
	if (device == NULL)
		return false;

	hapticDevice = device;
	return true;
}

/* Functions */
bool cBridge::openFileMapping(LPCSTR s) {
	fileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, s);

	if (fileMap == NULL) {
		errorMsg = "Invalid mapping file";
	}

	return !(fileMap == NULL);
}

bool cBridge::mapViewOfFiles() {
	// Check File Map
	if (fileMap == NULL) {
		errorMsg = "Invalid mapping file. Cannot open any view.";

		return false;
	}
	// Open View Files
	infoMapAddress = MapViewOfFile(fileMap, FILE_MAP_ALL_ACCESS, 0, 0, sysGran);
	if (infoMapAddress == NULL) {
		errorMsg = "Invalid file mapping address (information view)";
		return false;
	}

	iView = (InfoView *)infoMapAddress;
	sysGran = iView->sysGran;
	numberOfView = iView->numberOfView;
	sizeOfView = &(iView->viewInformation);//iView->sizeOfView;

	
	dataMapAddress = new LPVOID[numberOfView];
	oViews = new ObjectView*[numberOfView];
	//oViewData = new float*[numberOfView];

	int cumulatedSize = sysGran;
	
//#if TEST
	/// TEST CODES TO USE NEW STRUCTURE
	oNumViews = new ObjectNumberInView*[numberOfView];

	////////////////////// i = 0 
	////////////////////// Test view 0 -> ObjectEdit
	int i = 0;
	dataMapAddress[i] = MapViewOfFile(fileMap, FILE_MAP_ALL_ACCESS, 0, cumulatedSize, sizeOfView[i]);
	if (dataMapAddress[i] == NULL) {
		errorMsg = "Invalid file mapping address (data view)";
		return false;
	}
	oViews[i] = (ObjectView *)dataMapAddress[i];

	oNumViews[i] = (ObjectNumberInView *) &(oViews[i]->numberOfData);
	int dataNumber = oNumViews[i]->objectEditNumber;
	oEdits = new ObjectEdit*[dataNumber];
	
	
	float* dataAddress = (float *)(oNumViews[i]) + sizeof(ObjectNumberInView)/sizeof(float);
	int sizeOfStruct = sizeof(ObjectEdit);
	for (int j = 0; j < 2; j++) { // FIXME
		oEdits[j] = (ObjectEdit*)(dataAddress + (sizeOfStruct * j) / sizeof(float));
	}
	cumulatedSize += sizeOfView[i];

	////////////////////// i = 1
	////////////////////// Test view 1 -> ObjectConfiguration
	i = 1;
	dataMapAddress[i] = MapViewOfFile(fileMap, FILE_MAP_ALL_ACCESS, 0, cumulatedSize, sizeOfView[i]);
	if (dataMapAddress[i] == NULL) {
		errorMsg = "Invalid file mapping address (data view)";
		return false;
}
	oViews[i] = (ObjectView *)dataMapAddress[i];

	oNumViews[i] = (ObjectNumberInView *) &(oViews[i]->numberOfData);
	dataNumber = oNumViews[i]->objectConfigurationNumber;
	oConfigurations = new ObjectConfiguration*[dataNumber];
	objectScales = new cVector3d[dataNumber];
	
	dataAddress = (float *) (oNumViews[i]) + sizeof(ObjectNumberInView) / sizeof(float);
	sizeOfStruct = sizeof(ObjectConfiguration);
	for (int j = 0; j < dataNumber; j++) {
		oConfigurations[j] = (ObjectConfiguration*)(dataAddress + (sizeOfStruct * j) / sizeof(float));
	}
	cumulatedSize += sizeOfView[i];

//#else
	//for (int i = 0; i < 1; i++) {
	//	dataMapAddress[i] = MapViewOfFile(fileMap, FILE_MAP_ALL_ACCESS, 0, cumulatedSize, sizeOfView[i]);
	//	if (dataMapAddress[i] == NULL) {
	//		errorMsg = "Invalid file mapping address (data view)";
	//		return false;
	//	}
	//	oViews[i] = (ObjectView *)*dataMapAddress;
	//	//oViews[i]->data = (float *)(dataMapAddress + 4);
	//	//oViewData[i] = &(oViews[i]->data);//oViews[i]->data = &oViews[i]->data;

	//	int dataNumber = oViews[i]->numberOfData;
	//	oConfigurations = new ObjectConfiguration*[dataNumber];


	//	float* dataAddress = &(oViews[i]->data);
	//	int sizeOfStruct = sizeof(ObjectConfiguration);
	//	for (int j = 0; j < dataNumber; j++) {
	//		oConfigurations[j] = (ObjectConfiguration*)(dataAddress + (sizeOfStruct * j));
	//	}
	//	cumulatedSize += sizeOfView[i];
	//}
//#endif

	return true;
}

bool cBridge::checkNewModel(string & path, int* number) {
	bool isThereNewModels = false;
	int editNumber = oNumViews[0]->objectEditNumber;

	for (int i = 1; i > -1; i--) {
		//oEdits[i] = oEdits[i] & 0x00000000ffffffff;
		if ((long long)oEdits[i] < 0)
			oEdits[i] = (ObjectEdit*)((float*)(oNumViews[0]) + sizeof(ObjectNumberInView) / sizeof(float) + (sizeof(ObjectEdit) * i) / sizeof(float));
		int numberTag = oEdits[i]->objectNumTag;
		string fileName = oEdits[i]->filename;
		
		// if number tag is not exists in objectMap
		// register it 
		if (objectMap.find(numberTag) == objectMap.end()) {
			// register on map
			objectMap[numberTag] = NULL; 
		}
		else if (objectMap[numberTag] == NULL) {
		// if number tag has null memory then it is under loading or waiting.
		// underloading -> no need to do more.
			;
		}
		else {
		// No need to do anything more.
			continue;
		}

		/*string targetPath = filePaths[fileName];
		if (targetPath.size() == 0) {
			isThereNewModels = isThereNewModels || false;
			setErrorMsg("Invalid file name: " + fileName);
			continue;
		}*/

		if (*number == -1) {
			path = fileName;
			*number = numberTag;
		}

		isThereNewModels = true;
		setInfoMsg("Model loading...");
	}

	return isThereNewModels;
}

void cBridge::sendHIPData(float* HIP) {
	float scale = 1200;
	iView->HIP[0] = -HIP[0] * scale;
	iView->HIP[1] = HIP[1] * scale;
	iView->HIP[2] = HIP[2] * scale;
	return;
}

bool cBridge::uploadHIPData() {
	if (hapticDevice == NULL)
		return false;

	float scaleFactor = 1000; // FIXME: This is not final.
	cVector3d HIP;
	hapticDevice->getPosition(HIP);
	iView->HIP[0] = -HIP.x() * scaleFactor;
	iView->HIP[1] = HIP.y() * scaleFactor;
	iView->HIP[2] = HIP.z() * scaleFactor;

	return true;
}

bool cBridge::updateObject() {
	bool isSuccess = true;

	int sizeOfStruct;

	//int numberOfViews = iView->numberOfView;
	//for (int i = 0; i < 1; i++) {
	int numberOfData = oNumViews[1]->objectConfigurationNumber;
		// select structure here
		/*switch (viewInformation[i]->type) {
		case objectConfiguration:
			sizeOfStruct = sizeof(ObjectConfiguration);
			break;
		case objectDeformation:
			sizeOfStruct = sizeof(ObjectDeformation);
			break;
		}*/
		sizeOfStruct = sizeof(ObjectConfiguration);

		float* address = &(oViews[1]->data);
		for (int j = 0; j < numberOfData; j++) {
			cVector3d objPos;
			cMatrix3d objRot;
			cVector3d objScale;
			int objectNumber;
			isSuccess = isSuccess && getObjectData(j, &objectNumber, objPos, objRot, objScale);

			bool isNotLoaded = objectMap.find(objectNumber) == objectMap.end() || objectMap[objectNumber] == NULL;
			if (isNotLoaded)
				continue;

			cMultiMesh* currentObject = objectMap[objectNumber];
			currentObject->setLocalPos(objPos);
			currentObject->setLocalRot(objRot);
			if (objectScales[j].x() != objScale.x() ||
				objectScales[j].y() != objScale.y() ||
				objectScales[j].z() != objScale.z()) {
				//currentObject->scale(objScale.x());
				objectScales[j] = objScale;
			}
			//currentObject->scale(objScale.x());
			//currentObject->scaleXYZ(objScale.x(), objScale.y(), objScale.z());
		//}
	}

	return isSuccess;
}

bool cBridge::getObjectData(int objNum, int *number, cVector3d& pos, cMatrix3d& rot, cVector3d& scale) {
	// out of bound -> goes wrong
	if (objNum >= oViews[0]->numberOfData)
		return false;

	// FIXME: current scale factor is not right
	double scaleFactor = 0.001;

	// Set this object's number
	*number = oConfigurations[objNum]->objectTag;

	// Set position data of some object
	const double posX = (oConfigurations[objNum]->objectPositionX);
	const double posY = (-oConfigurations[objNum]->objectPositionY);
	const double posZ = (oConfigurations[objNum]->objectPositionZ);
	pos.set(posX * scaleFactor, posY * scaleFactor, posZ * scaleFactor);

	// Set rotation data of some object
	const double rotX = oConfigurations[objNum]->objectRotationRoll;
	const double rotY = -oConfigurations[objNum]->objectRotationPitch;
	const double rotZ = -oConfigurations[objNum]->objectRotationYaw;
	rot.setExtrinsicEulerRotationDeg(rotX, rotY, rotZ, C_EULER_ORDER_XYZ);

	// Set scale data of some object
	scale.x(oConfigurations[objNum]->objectScaleX);
	scale.y(oConfigurations[objNum]->objectScaleY);
	scale.z(oConfigurations[objNum]->objectScaleZ);
	
	return true;
}

/* Current status functions */
string cBridge::getCurrentPosition(int i) {
	ObjectConfiguration* data = oConfigurations[i];
	string result = "";
/*
	if (data != NULL)
		result = cStr(data->objectPositionX, 3) + ", "
		+ cStr(data->objectPositionY, 3) + ", "
		+ cStr(data->objectPositionZ, 3);
	else
		result = "Failed to get object configuration";*/

	return result;
}

string cBridge::getCurrentRotation(int i) {
	ObjectConfiguration* data = oConfigurations[i];
	string result = "";
/*
	if (data != NULL)
		result = cStr(data->objectRotationRoll, 3) + ", "
		+ cStr(data->objectRotationPitch, 3) + ", "
		+ cStr(data->objectRotationYaw, 3);
	else
		result = "Failed to get object configuration";*/

	return result;
}

string cBridge::getCurrentScale(int i) {
	ObjectConfiguration* data = oConfigurations[i];
	string result;

	if (data != NULL)
		result = cStr(data->objectScaleX, 3) + ", "
		+ cStr(data->objectScaleY, 3) + ", "
		+ cStr(data->objectScaleZ, 3);
	else
		result = "Failed to get object configuration";

	return result;
}

string cBridge::getInformationMessage() {
	return infoMsg;
}

string cBridge::getErrorMessage() {
	return errorMsg;
}

void cBridge::setInfoMsg(string info) {
	infoMsg = info;
}

void cBridge::setErrorMsg(string error) {
	errorMsg = error;
}


}