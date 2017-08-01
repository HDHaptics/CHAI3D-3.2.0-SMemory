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
		
		cumulatedSize += sizeOfView[i];
	}

	return true;
}

void cBridge::sendHIPData(float* HIP) {
	float scale = 10000;
	iView->HIP[0] = HIP[0] * scale;
	iView->HIP[1] = HIP[1] * scale;
	iView->HIP[2] = HIP[2] * scale;
	return;
}

bool cBridge::getObjectData(int objNum, cVector3d& pos, cVector3d& rot) {
	int currentLocation = objNum * 6;
	pos.x(oViewData[0][currentLocation + 0]);
	pos.y(oViewData[0][currentLocation + 1]);
	pos.z(oViewData[0][currentLocation + 2]);
	rot.x(oViewData[0][currentLocation + 3]);
	rot.y(oViewData[0][currentLocation + 4]);
	rot.z(oViewData[0][currentLocation + 5]);
	return true;
}

}