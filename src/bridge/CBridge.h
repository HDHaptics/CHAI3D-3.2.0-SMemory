//==============================================================================
/*

\author    Hojun Cha
\version   0.0.1
*/
//==============================================================================

#ifndef CBridgeH
#define CBridgeH

#include "chai3d.h"

using namespace std;
namespace chai3d {

typedef struct {
	int sysGran;
	float HIP[3];
	int numberOfView;
	int sizeOfView;
} InfoView;

typedef struct {
	int numberOfData;
	float data;
} ObjectView;

typedef struct {
	float objectPositionX;
	float objectPositionY;
	float objectPositionZ;
	float objectRotationRoll;
	float objectRotationPitch;
	float objectRotationYaw;
	float objectScaleX;
	float objectScaleY;
	float objectScaleZ;
} ObjectConfiguration;

class cBridge {
public:
	/* Register important components */
	cGenericHapticDevicePtr hapticDevice;
	vector<cMultiMesh *> objects;

	HANDLE fileMap;
	LPVOID infoMapAddress;
	LPVOID* dataMapAddress;

	SIZE_T sizeObjNum;
	SIZE_T sizeObjData;
	int sysGran;

	InfoView* iView;
	int numberOfView;
	int* sizeOfView;
	ObjectView** oViews;
	ObjectConfiguration** oConfigurations;
	float** oViewData;

	SYSTEM_INFO sysInfo;
	LPCSTR errorMsg;

	int currentViewNumber;

	/* Constructor */
	cBridge();
	
	/* Grouping functions */
	bool Tick();

	/* Registering function */
	bool registerHapticDevice(cGenericHapticDevicePtr);
	bool registerObjects(vector<cMultiMesh *>);

	/* Functions */
	bool openFileMapping(LPCSTR s);

	bool mapViewOfFiles();

	void sendHIPData(float*);
	bool uploadHIPData();

	bool updateObject();
	bool getObjectData(int, cVector3d&, cMatrix3d&, cVector3d&);
};

}

#endif // !CBridgeH