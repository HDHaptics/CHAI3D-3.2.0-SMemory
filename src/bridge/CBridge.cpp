//==============================================================================
/*

\author    Hojun Cha
\version   0.0.1
*/
//==============================================================================

//#include "CBridge.h"
#include "bridge/CBridge.h"

using namespace std;
namespace chai3D {

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

	if (fileMap == NULL) {
		errorMsg = "Invalid mapping file. Cannot open any view.";

		return false;
	}

	infoMapAddress = MapViewOfFile(fileMap, FILE_MAP_ALL_ACCESS, 0, 0, sysGran);
	if (infoMapAddress == NULL) {
		errorMsg = "Invalid file mapping address.";
		return false;
	}

	Iview = (InfoView *)infoMapAddress;
	numberOfView = Iview->numberOfView;
	sizeOfView = Iview->sizeOfView;
	dataMapAddress = new LPVOID[numberOfView];

}

}