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

class cBridge {
public:
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
	float** oViewData;

	SYSTEM_INFO sysInfo;
	LPCSTR errorMsg;

	int currentViewNumber;

	cBridge();

	bool openFileMapping(LPCSTR s);

	bool mapViewOfFiles();

	void sendHIPData(float*);
};

}

#endif // !CBridgeH