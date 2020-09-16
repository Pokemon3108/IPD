#include <stdio.h>
#include <Windows.h>
#include <SetupAPI.h>
#include <locale.h>

#pragma comment (lib, "Setupapi.lib")

#define buffersize 512

int main() {
	setlocale(LC_ALL, "Russian");

	const char* INTERFACE_NAME = "PCI";

	int deviceIndex = 0;
	
	HDEVINFO hDevicesInfo = SetupDiGetClassDevs(NULL, INTERFACE_NAME, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT); //handle to a device info set
	if (hDevicesInfo == INVALID_HANDLE_VALUE) {
		puts("Getting devices failed");
		return 0;
	}

	SP_DEVINFO_DATA deviceData; //specify info for device from device info set
	deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

	while (SetupDiEnumDeviceInfo(hDevicesInfo, deviceIndex, &deviceData)) {
		int realSize = 0;
		char name[buffersize];

		//vendor
		SetupDiGetDeviceRegistryProperty(hDevicesInfo, &deviceData, SPDRP_MFG, NULL, name, buffersize, &realSize); //get selected info about device 
		//according to  flag
		
		printf("Vendor: %s\n", name);

		//device
		SetupDiGetDeviceRegistryProperty(hDevicesInfo, &deviceData, SPDRP_DEVICEDESC,NULL, name, buffersize, &realSize);
		printf("Device: %s\n\n", name);

		++deviceIndex;
	}


	SetupDiDestroyDeviceInfoList(hDevicesInfo);
	system("pause");
	return 0;


}

