//==============================================================================
/*

\author    Hojun Cha
\version   0.0.1
*/
//==============================================================================

#ifndef CBridgeH
#define CBridgeH

using namespace std;
namespace chai3D {

typedef struct {
	int sysGran;
	int numberOfView;
	int* sizeOfView;
} InfoView;

typedef struct {
	int numberOfData;
	float* data;
} ObjectView;

class cBridge {
public:
	HANDLE fileMap;
	LPVOID infoMapAddress;
	LPVOID* dataMapAddress;

	SIZE_T sizeObjNum;
	SIZE_T sizeObjData;
	DWORD sysGran;

	InfoView* Iview;
	int numberOfView;
	int* sizeOfView;
	ObjectView* Oview;

	SYSTEM_INFO sysInfo;
	LPCSTR errorMsg;

	int currentViewNumber;

	cBridge();

	bool openFileMapping(LPCSTR s);

	bool mapViewOfFiles();
};

}

#endif // !CBridgeH