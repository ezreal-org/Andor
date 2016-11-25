#include "atcore.h"
#include <iostream> 
using namespace std;

int main(int argc, char* argv[])
{
	int i_retCode;
	cout << "Initialising ..." << endl << endl;
	i_retCode = AT_InitialiseLibrary();
	AT_64 iNumberDevices = 0;
	AT_H Hndl;
	AT_GetInt(AT_HANDLE_SYSTEM, L"Device Count", &iNumberDevices);

	for (int i = 0; i < iNumberDevices; i++) {
		i_retCode = AT_Open(i, &Hndl);
		AT_WC szValue[64];
		i_retCode = AT_GetString(Hndl, L"Serial Number", szValue, 64);
		wcout << L"The serial number is " << szValue << endl;
		AT_Close(Hndl);
	}

	AT_FinaliseLibrary();
	cout << endl << "Press any key then enter to close" << endl;
	char ch;
	cin >> ch;

	return 0;
}


