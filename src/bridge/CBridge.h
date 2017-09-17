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

struct InfoView{
	int sysGran;
	float HIP[3];
	int numberOfView;
	int viewInformation;

	InfoView() {
		sysGran = 65536;
		HIP[0] = 0;
		HIP[1] = 0;
		HIP[2] = 0;
		numberOfView = 1;
		viewInformation = 0;
	}
};

struct ObjectView {
	int numberOfData;
	float data;
	
	ObjectView() {
		numberOfData = 0;
		data = 0;
	}
} ;

struct ObjectConfiguration {
	int objectTag;
	float objectPositionX;
	float objectPositionY;
	float objectPositionZ;
	float objectRotationRoll;
	float objectRotationPitch;
	float objectRotationYaw;
	float objectScaleX;
	float objectScaleY;
	float objectScaleZ;

	ObjectConfiguration() {
		objectTag = 0;
		objectPositionX = 0;
		objectPositionY = 0;
		objectPositionZ = 0;
		objectRotationRoll = 0;
		objectRotationPitch = 0;
		objectRotationYaw = 0;
		objectScaleX = 1;
		objectScaleY = 1;
		objectScaleZ = 1;
	}
};

struct ObjectEdit {
	bool command;
	int objectNumTag;
	char filename[200];

	ObjectEdit() {
		command = 0;
		objectNumTag = 1;
	}
};

struct ObjectNumberInView {
	int objectConfigurationNumber;
	int verticesNumber;
	int objectEditNumber;

	ObjectNumberInView () {
		objectConfigurationNumber = 0;
		verticesNumber = 0;
		objectEditNumber = 0;
	}
};

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
//#if TEST
	ObjectNumberInView** oNumViews;
	ObjectEdit** oEdits;
//#endif
	ObjectConfiguration** oConfigurations;
	float** oViewData;
	cVector3d* objectScales;

	map<string, string> filePaths;
	map<int, cMultiMesh*> objectMap;
	//vector<int> objectLoadingLists;

	/* Messages */
	SYSTEM_INFO sysInfo;
	string errorMsg;
	string infoMsg;

	int currentViewNumber;

	/* Constructor */
	cBridge();
	
	/* Grouping functions */
	bool Tick();

	/* Registering functions */
	bool registerHapticDevice(cGenericHapticDevicePtr);
	bool registerObject(int, cMultiMesh*);
	/*bool registerObjects(int, vector<cMultiMesh *>);
	vector<cMultiMesh*> getObjects();*/
	bool updateFileList();
	bool registerFileInFolder(string);

	/* Functions */
	bool openFileMapping(LPCSTR s);

	bool mapViewOfFiles();

	bool checkNewModel(string &, int*);

	void sendHIPData(float*);
	bool uploadHIPData();

	bool updateObject();
	bool getObjectData(int, int*, cVector3d&, cMatrix3d&, cVector3d&);

	/* Current status functions */
	string getCurrentPosition(int);
	string getCurrentRotation(int);
	string getCurrentScale(int);
	string getInformationMessage();
	string getErrorMessage();
	void setInfoMsg(string);
	void setErrorMsg(string);

	/* Model loading thread */
	//void loadModels(void);
};

}

#endif // !CBridgeH