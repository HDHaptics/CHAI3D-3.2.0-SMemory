//==============================================================================
/*

\author    Hojun Cha
\version   0.0.1
*/
//==============================================================================

#include "bridge/CBridge.h"

using namespace std;
namespace chai3d {

cBridge::cBridge() {
	currentViewNumber = 0;
	numberOfView = 0;
	sizeOfView = new int[0]{};
}

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
	oViewData = new float*[numberOfView];

	int cumulatedSize = sysGran;
	
	for (int i = 0; i < 1; i++) {
		dataMapAddress[i] = MapViewOfFile(fileMap, FILE_MAP_ALL_ACCESS, 0, cumulatedSize, sizeOfView[i]);
		if (dataMapAddress[i] == NULL) {
			errorMsg = "Invalid file mapping address (data view)";
			return false;
		}
		oViews[i] = (ObjectView *)*dataMapAddress;
		//oViews[i]->data = (float *)(dataMapAddress + 4);
		oViewData[i] = &(oViews[i]->data);//oViews[i]->data = &oViews[i]->data;
		
		int dataNumber = oViews[i]->numberOfData;
		oPrimitives = new ObjectPrimitive*[dataNumber];
		
		
		float* dataAddress = &(oViews[i]->data);
		int sizeOfStruct = sizeof(ObjectPrimitive);
		for (int j = 0; j < dataNumber; j++) {
			oPrimitives[j] = (ObjectPrimitive*) (dataAddress + (sizeOfStruct * j));
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


bool cBridge::getObjectData(int objNum, cVector3d& pos, cMatrix3d& rot, cVector3d& scale) {
	// out of bound -> goes wrong
	if (objNum >= oViews[0]->numberOfData)
		return false;

	// FIXME: current scale factor is not right
	double scaleFactor = 0.003;

	// Get position data of some object
	const double posX = (oPrimitives[objNum]->objectPositionX);
	const double posY = (-oPrimitives[objNum]->objectPositionY);
	const double posZ = (oPrimitives[objNum]->objectPositionZ);
	pos.set(posX * scaleFactor, posY * scaleFactor, posZ * scaleFactor);

	// Set rotation data of some object
	const double rotX = oPrimitives[objNum]->objectRotationRoll;
	const double rotY = -oPrimitives[objNum]->objectRotationPitch;
	const double rotZ = -oPrimitives[objNum]->objectRotationYaw;
	rot.setExtrinsicEulerRotationDeg(rotX, rotY, rotZ, C_EULER_ORDER_XYZ);

	// Set scale data of some object
	scale.x(oPrimitives[objNum]->objectScaleX);
	scale.y(oPrimitives[objNum]->objectScaleY);
	scale.z(oPrimitives[objNum]->objectScaleZ);

	return true;
}

}