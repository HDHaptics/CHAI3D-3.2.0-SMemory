//==============================================================================
/*

\author    Hojun Cha
\version   0.0.1
*/
//==============================================================================

#include "bridge/CBridge.h"

using namespace std;
namespace chai3d {
	
/* Constructor */
cBridge::cBridge() {
	currentViewNumber = 0;
	numberOfView = 0;
	sizeOfView = new int[0]{};
}

/* Grouping functions */
bool cBridge::Tick() {
	bool isSuccess = true;

	if (!uploadHIPData()) {
		isSuccess = false;
		errorMsg = "Fail to upload HIP Info.";
	}
		
	return true;
}

bool cBridge::registerObjects(vector<cMultiMesh *> obj) {
	objects = obj;
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
	sizeOfView = &(iView->sizeOfView);//iView->sizeOfView;

	
	dataMapAddress = new LPVOID[numberOfView];
	oViews = new ObjectView*[numberOfView];
	//oViewData = new float*[numberOfView];

	int cumulatedSize = sysGran;
	
	for (int i = 0; i < 1; i++) {
		dataMapAddress[i] = MapViewOfFile(fileMap, FILE_MAP_ALL_ACCESS, 0, cumulatedSize, sizeOfView[i]);
		if (dataMapAddress[i] == NULL) {
			errorMsg = "Invalid file mapping address (data view)";
			return false;
		}
		oViews[i] = (ObjectView *)*dataMapAddress;
		//oViews[i]->data = (float *)(dataMapAddress + 4);
		//oViewData[i] = &(oViews[i]->data);//oViews[i]->data = &oViews[i]->data;
		
		int dataNumber = oViews[i]->numberOfData;
		oConfigurations = new ObjectConfiguration*[dataNumber];
		
		
		float* dataAddress = &(oViews[i]->data);
		int sizeOfStruct = sizeof(ObjectConfiguration);
		for (int j = 0; j < dataNumber; j++) {
			oConfigurations[j] = (ObjectConfiguration*) (dataAddress + (sizeOfStruct * j));
		}
		cumulatedSize += sizeOfView[i];
	}

	return true;
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

	float scaleFactor = 1200; // FIXME: This is not final.
	cVector3d HIP;
	hapticDevice->getPosition(HIP);
	iView->HIP[0] = -HIP.x * scaleFactor;
	iView->HIP[1] = HIP.y * scaleFactor;
	iView->HIP[2] = HIP.z * scaleFactor;

	return true;
}

bool cBridge::updateObject() {
	
	cVector3d position;
	cMatrix3d rotation;
	cVector3d scale;
	int sizeOfStruct;

	int numberOfViews = iView->numberOfView;
	for (int i = 0; i < numberOfView; i++) {
		int numberOfData = oViews[i]->numberOfData;
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

		float* address = &(oViews[i]->data);
		for (int j = 0; j < numberOfData; j++) {
			cVector3d objPos;
			cMatrix3d objRot;
			cVector3d objScale;
			getObjectData(j, objPos, objRot, objScale);

			objects[j]->setLocalPos(objPos);
			objects[j]->setLocalRot(objRot);
			//objects[j]->scaleXYZ(objScale.x, objScale.y, objScale.z);
		}
	}
}

bool cBridge::getObjectData(int objNum, cVector3d& pos, cMatrix3d& rot, cVector3d& scale) {
	// out of bound -> goes wrong
	if (objNum >= oViews[0]->numberOfData)
		return false;

	// FIXME: current scale factor is not right
	double scaleFactor = 0.003;

	// Get position data of some object
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

}