#include <iostream>
#include <Windows.h>
#include <string> 
#include <iomanip> 
#include <map>
#include <tchar.h>

#include "devioctl.h"
#include "ntdddisk.h"

#define _WIN32_WINNT 0x500
#pragma warning(disable:4786)

using namespace std;
typedef map<DWORD, LPVOID> SMARTINFOMAP;

#define SMART_ATTRIB_RAW_READ_ERROR_RATE                    1
#define SMART_ATTRIB_THROUGHPUT_PERFORMANCE                 2
#define SMART_ATTRIB_SPIN_UP_TIME                           3
#define SMART_ATTRIB_START_STOP_COUNT                       4
#define SMART_ATTRIB_START_REALLOCATION_SECTOR_COUNT        5
#define SMART_ATTRIB_SEEK_ERROR_RATE                        7
#define SMART_ATTRIB_POWER_ON_HOURS_COUNT                   9
#define SMART_ATTRIB_SPIN_RETRY_COUNT                       10
#define SMART_ATTRIB_RECALIBRATION_RETRIES                  11
#define SMART_ATTRIB_DEVICE_POWER_CYCLE_COUNT               12
#define SMART_ATTRIB_SOFT_READ_ERROR_RATE                   13
#define SMART_ATTRIB_LOAD_UNLOAD_CYCLE_COUNT                193
#define SMART_ATTRIB_TEMPERATURE                            194
#define SMART_ATTRIB_ECC_ON_THE_FLY_COUNT                   195
#define SMART_ATTRIB_REALLOCATION_EVENT_COUNT               196
#define SMART_ATTRIB_CURRENT_PENDING_SECTOR_COUNT           197
#define SMART_ATTRIB_UNCORRECTABLE_SECTOR_COUNT             198
#define SMART_ATTRIB_ULTRA_DMA_CRC_ERROR_COUNT              199
#define SMART_ATTRIB_WRITE_ERROR_RATE                       200
#define SMART_ATTRIB_TA_COUNTER_INCREASED                   202
#define SMART_ATTRIB_GSENSE_ERROR_RATE                      221
#define SMART_ATTRIB_POWER_OFF_RETRACT_COUNT                228
#define MAX_ATTRIBUTES  256

#define INDEX_ATTRIB_INDEX                                  0
#define INDEX_ATTRIB_UNKNOWN1                               1
#define INDEX_ATTRIB_UNKNOWN2                               2
#define INDEX_ATTRIB_VALUE                                  3
#define INDEX_ATTRIB_WORST                                  4
#define INDEX_ATTRIB_RAW                                    5

//SMART structures

typedef struct
{
	BYTE m_ucAttribIndex;
	DWORD m_dwAttribValue;
	BYTE m_ucValue;
	BYTE m_ucWorst;
	DWORD m_dwThreshold;
}ST_SMART_INFO;


typedef struct
{
	WORD wGenConfig;
	WORD wNumCyls;
	WORD wReserved;
	WORD wNumHeads;
	WORD wBytesPerTrack;
	WORD wBytesPerSector;
	WORD wSectorsPerTrack;
	WORD wVendorUnique[3];
	BYTE sSerialNumber[20];
	WORD wBufferType;
	WORD wBufferSize;
	WORD wECCSize;
	BYTE sFirmwareRev[8];
	BYTE sModelNumber[39];
	WORD wMoreVendorUnique;
	WORD wDoubleWordIO;
	WORD wCapabilities;
	WORD wReserved1;
	WORD wPIOTiming;
	WORD wDMATiming;
	WORD wBS;
	WORD wNumCurrentCyls;
	WORD wNumCurrentHeads;
	WORD wNumCurrentSectorsPerTrack;
	WORD ulCurrentSectorCapacity;
	WORD wMultSectorStuff;
	DWORD ulTotalAddressableSectors;
	WORD wSingleWordDMA;
	WORD wMultiWordDMA;
	BYTE bReserved[127];
}ST_IDSECTOR;

typedef struct
{
	BYTE  bDriverError;
	BYTE  bIDEStatus;
	BYTE  bReserved[2];
	DWORD dwReserved[2];
} ST_DRIVERSTAT;

typedef struct
{
	DWORD      cBufferSize;
	ST_DRIVERSTAT DriverStatus;
	BYTE       bBuffer[1];
} ST_ATAOUTPARAM;


typedef struct
{
	GETVERSIONINPARAMS m_stGVIP;
	ST_IDSECTOR m_stInfo;
	ST_SMART_INFO m_stSmartInfo[256];
	BYTE m_ucSmartValues;
	BYTE m_ucDriveIndex;
	char m_csErrorString[1000];
}ST_DRIVE_INFO;

typedef struct
{
	BOOL m_bCritical;
	BYTE m_ucAttribId;
	char m_csAttribName[100];
	char m_csAttribDetails[1000];
}ST_SMART_DETAILS;

#define DRIVE_HEAD_REG  0xA0


SMARTINFOMAP m_oSmartInfo; //map для хранения SMART-показателей
ST_DRIVE_INFO m_stDrivesInfo[32]; //массив с данными о жестких дисках


//преобразование идентификатора SMART-показателя в строку
const char* SmartIndexToString(BYTE index)
{
	switch (index)
	{
	case SMART_ATTRIB_RAW_READ_ERROR_RATE: return "RAW_READ_ERROR_RATE";
	case  SMART_ATTRIB_THROUGHPUT_PERFORMANCE: return "THROUGHPUT_PERFORMANCE";
	case  SMART_ATTRIB_SPIN_UP_TIME: return "SPIN_UP_TIME";
	case  SMART_ATTRIB_START_STOP_COUNT: return "START_STOP_COUNT";
	case  SMART_ATTRIB_START_REALLOCATION_SECTOR_COUNT: return "START_REALLOCATION_SECTOR_COUNT";
	case  SMART_ATTRIB_SEEK_ERROR_RATE: return "SEEK_ERROR_RATE";
	case  SMART_ATTRIB_POWER_ON_HOURS_COUNT: return "POWER_ON_HOURS_COUNT";
	case  SMART_ATTRIB_SPIN_RETRY_COUNT: return "SPIN_RETRY_COUNT";
	case  SMART_ATTRIB_RECALIBRATION_RETRIES: return "RECALIBRATION_RETRIES";
	case  SMART_ATTRIB_DEVICE_POWER_CYCLE_COUNT: return "DEVICE_POWER_CYCLE_COUNT";
	case  SMART_ATTRIB_SOFT_READ_ERROR_RATE: return "SOFT_READ_ERROR_RATE";
	case  SMART_ATTRIB_LOAD_UNLOAD_CYCLE_COUNT: return "LOAD_UNLOAD_CYCLE_COUNT";
	case  SMART_ATTRIB_TEMPERATURE: return "TEMPERATURE";
	case  SMART_ATTRIB_ECC_ON_THE_FLY_COUNT: return "ECC_ON_THE_FLY_COUNT";
	case  SMART_ATTRIB_REALLOCATION_EVENT_COUNT: return "REALLOCATION_EVENT_COUNT";
	case  SMART_ATTRIB_CURRENT_PENDING_SECTOR_COUNT: return "CURRENT_PENDING_SECTOR_COUNT";
	case  SMART_ATTRIB_UNCORRECTABLE_SECTOR_COUNT: return "UNCORRECTABLE_SECTOR_COUNT";
	case  SMART_ATTRIB_ULTRA_DMA_CRC_ERROR_COUNT: return "ULTRA_DMA_CRC_ERROR_COUNT";
	case  SMART_ATTRIB_WRITE_ERROR_RATE: return "WRITE_ERROR_RATE  ";
	case  SMART_ATTRIB_TA_COUNTER_INCREASED: return "TA_COUNTER_INCREASED";
	case  SMART_ATTRIB_GSENSE_ERROR_RATE: return "GSENSE_ERROR_RATE";
	case  SMART_ATTRIB_POWER_OFF_RETRACT_COUNT: return "POWER_OFF_RETRACT_COUNT   ";

	default:
		return "";
	}
}

//вывод сообщения об ошибке
void ErrorMes(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	printf("%s failed with error %d: %s",
		lpszFunction, dw, lpMsgBuf);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);

}

//получение значения показателя из m_oSmartInfo
ST_SMART_INFO * GetSMARTValue(BYTE ucDriveIndex, BYTE ucAttribIndex)
{
	SMARTINFOMAP::iterator pIt;
	ST_SMART_INFO *pRet = NULL;

	pIt = m_oSmartInfo.find(MAKELPARAM(ucAttribIndex, ucDriveIndex));
	if (pIt != m_oSmartInfo.end())
		pRet = (ST_SMART_INFO *)pIt->second;
	return pRet;
}

BOOL ReadSMARTAttributes(HANDLE hDevice, UCHAR ucDriveIndex)
{
	SENDCMDINPARAMS stCIP = { 0 };
	DWORD dwRet = 0;
	BOOL bRet = FALSE;
	BYTE szAttributes[sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1];
	UCHAR ucT1;
	PBYTE pT1, pT3; PDWORD pT2;
	ST_SMART_INFO *pSmartValues;

	stCIP.cBufferSize = READ_ATTRIBUTE_BUFFER_SIZE;
	stCIP.bDriveNumber = ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg = READ_ATTRIBUTES;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	stCIP.irDriveRegs.bCylHighReg = SMART_CYL_HI;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = SMART_CMD;
	bRet = DeviceIoControl(hDevice, SMART_RCV_DRIVE_DATA, &stCIP, sizeof(stCIP), szAttributes, sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1, &dwRet, NULL);
	if (bRet)
	{
		m_stDrivesInfo[ucDriveIndex].m_ucSmartValues = 0;
		m_stDrivesInfo[ucDriveIndex].m_ucDriveIndex = ucDriveIndex;
		pT1 = (PBYTE)(((ST_ATAOUTPARAM*)szAttributes)->bBuffer);
		for (ucT1 = 0; ucT1 < 30; ++ucT1)
		{
			pT3 = &pT1[2 + ucT1 * 12];
			pT2 = (PDWORD)&pT3[INDEX_ATTRIB_RAW];
			pT3[INDEX_ATTRIB_RAW + 2] = pT3[INDEX_ATTRIB_RAW + 3] = pT3[INDEX_ATTRIB_RAW + 4] = pT3[INDEX_ATTRIB_RAW + 5] = pT3[INDEX_ATTRIB_RAW + 6] = 0;
			if (pT3[INDEX_ATTRIB_INDEX] != 0)
			{
				pSmartValues = &m_stDrivesInfo[ucDriveIndex].m_stSmartInfo[m_stDrivesInfo[ucDriveIndex].m_ucSmartValues];
				pSmartValues->m_ucAttribIndex = pT3[INDEX_ATTRIB_INDEX];
				pSmartValues->m_ucValue = pT3[INDEX_ATTRIB_VALUE];
				pSmartValues->m_ucWorst = pT3[INDEX_ATTRIB_WORST];
				pSmartValues->m_dwAttribValue = pT2[0];
				pSmartValues->m_dwThreshold = MAXDWORD;
				m_oSmartInfo[MAKELPARAM(pSmartValues->m_ucAttribIndex, ucDriveIndex)] = pSmartValues;
				m_stDrivesInfo[ucDriveIndex].m_ucSmartValues++;
			}
		}
	}
	else dwRet = GetLastError();

	stCIP.irDriveRegs.bFeaturesReg = READ_THRESHOLDS;
	stCIP.cBufferSize = READ_THRESHOLD_BUFFER_SIZE; // Is same as attrib size
	bRet = DeviceIoControl(hDevice, SMART_RCV_DRIVE_DATA, &stCIP, sizeof(stCIP), szAttributes, sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1, &dwRet, NULL);
	if (bRet)
	{
		pT1 = (PBYTE)(((ST_ATAOUTPARAM*)szAttributes)->bBuffer);
		for (ucT1 = 0; ucT1 < 30; ++ucT1)
		{
			pT2 = (PDWORD)&pT1[2 + ucT1 * 12 + 5];
			pT3 = &pT1[2 + ucT1 * 12];
			pT3[INDEX_ATTRIB_RAW + 2] = pT3[INDEX_ATTRIB_RAW + 3] = pT3[INDEX_ATTRIB_RAW + 4] = pT3[INDEX_ATTRIB_RAW + 5] = pT3[INDEX_ATTRIB_RAW + 6] = 0;
			if (pT3[0] != 0)
			{
				pSmartValues = GetSMARTValue(ucDriveIndex, pT3[0]);
				if (pSmartValues)
					pSmartValues->m_dwThreshold = pT2[0];
			}
		}
	}
	return bRet;
}

BOOL CollectDriveInfo(HANDLE hDevice, UCHAR ucDriveIndex)
{
	BOOL bRet = FALSE;
	SENDCMDINPARAMS stCIP = { 0 };
	DWORD dwRet = 0;
#define OUT_BUFFER_SIZE IDENTIFY_BUFFER_SIZE+16
	char szOutput[OUT_BUFFER_SIZE] = { 0 };

	stCIP.cBufferSize = IDENTIFY_BUFFER_SIZE;
	stCIP.bDriveNumber = ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg = 0;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = 0;
	stCIP.irDriveRegs.bCylHighReg = 0;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = ID_CMD;

	bRet = DeviceIoControl(hDevice, SMART_RCV_DRIVE_DATA, &stCIP, sizeof(stCIP), szOutput, OUT_BUFFER_SIZE, &dwRet, NULL);
	if (bRet)
	{
		CopyMemory(&m_stDrivesInfo[ucDriveIndex].m_stInfo, szOutput + 16, sizeof(ST_IDSECTOR));
	}
	else
		dwRet = GetLastError();
	return bRet;
}

BOOL IsSmartEnabled(HANDLE hDevice, UCHAR ucDriveIndex)
{
	SENDCMDINPARAMS stCIP = { 0 };
	SENDCMDOUTPARAMS stCOP = { 0 };
	DWORD dwRet = 0;
	BOOL bRet = FALSE;

	stCIP.cBufferSize = 0;
	stCIP.bDriveNumber = ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg = ENABLE_SMART;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	stCIP.irDriveRegs.bCylHighReg = SMART_CYL_HI;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = SMART_CMD;

	bRet = DeviceIoControl(hDevice, SMART_SEND_DRIVE_COMMAND, &stCIP, sizeof(stCIP), &stCOP, sizeof(stCOP), &dwRet, NULL);
	if (bRet)
	{

	}
	else
	{
		dwRet = GetLastError();
		printf(m_stDrivesInfo[ucDriveIndex].m_csErrorString, "Error %d in reading SMART Enabled flag", dwRet);
	}
	return bRet;
}

//Считывает SMART-показатели для диска с указанным индексом
BOOL ReadSMARTInfo(BYTE ucDriveIndex)
{
	HANDLE hDevice = NULL;
	char szT1[MAX_PATH] = { 0 };
	BOOL bRet = FALSE;
	DWORD dwRet = 0;

	wsprintf(szT1, "\\\\.\\PHYSICALDRIVE%d", ucDriveIndex);
	hDevice = CreateFile(szT1, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		bRet = DeviceIoControl(hDevice, SMART_GET_VERSION, NULL, 0, &m_stDrivesInfo[ucDriveIndex].m_stGVIP, sizeof(GETVERSIONINPARAMS), &dwRet, NULL);
		if (bRet)
		{
			if ((m_stDrivesInfo[ucDriveIndex].m_stGVIP.fCapabilities & CAP_SMART_CMD) == CAP_SMART_CMD)
			{
				if (IsSmartEnabled(hDevice, ucDriveIndex))
				{
					bRet = CollectDriveInfo(hDevice, ucDriveIndex);
					bRet = ReadSMARTAttributes(hDevice, ucDriveIndex);
				}
			}
		}
		CloseHandle(hDevice);
	}
	
	return bRet;
}

const char* bus_types[] = {
	"UNKNOWN",
	"SCSI",
	"ATAPI",
	"ATA",
	"ONE_TREE_NINE_FOUR",
	"SSA",
	"FIBRE",
	"USB",
	"RAID",
	"ISCSI",
	"SAS",
	"SATA",
	"SD",
	"MMC"
	"VIRTUAL",
	"FILE_BACKED_VIRTUAL"
};

HANDLE getDriveHandle(int driveNumber) {
	std::string drive = "\\\\.\\PhysicalDrive" + std::to_string(driveNumber);
	return CreateFileA(drive.c_str(), 0, 0, NULL, OPEN_EXISTING, 0, NULL);
}

void getDriveInfo(HANDLE hDevice) {
	using namespace std;

	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = { 0 };

	STORAGE_PROPERTY_QUERY propertyQuery;
	propertyQuery.PropertyId = StorageDeviceProperty;
	propertyQuery.QueryType = PropertyStandardQuery;

	
	DeviceIoControl(hDevice,
					IOCTL_STORAGE_QUERY_PROPERTY,
					&propertyQuery,
					sizeof(STORAGE_PROPERTY_QUERY),
					&storageDescriptorHeader,
					sizeof(STORAGE_DESCRIPTOR_HEADER),
					NULL,
					NULL);


	int bufferSize = storageDescriptorHeader.Size;
	char* buffer = new char[bufferSize];
	ZeroMemory(buffer, bufferSize);


	DeviceIoControl(hDevice,
					IOCTL_STORAGE_QUERY_PROPERTY,
					&propertyQuery,
					sizeof(STORAGE_PROPERTY_QUERY),
					buffer,
					bufferSize,
					NULL,
					NULL);
	

	STORAGE_DEVICE_DESCRIPTOR* storageDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)buffer;
	storageDeviceDescriptor->Size = bufferSize;

	if (storageDeviceDescriptor->VendorIdOffset != 0)
	{
		cout << "Vendor: " <<  buffer+ storageDeviceDescriptor->VendorIdOffset << endl;
	}
	if (storageDeviceDescriptor->ProductIdOffset != 0)
	{
		cout << "Model: " << buffer + storageDeviceDescriptor->ProductIdOffset << endl;
	}
	if (storageDeviceDescriptor->ProductRevisionOffset != 0)
	{
		cout << "Firmware version: " << buffer + storageDeviceDescriptor->ProductRevisionOffset <<endl;
	}
	if (storageDeviceDescriptor->BusType != 0)
	{
		cout << "Bus type: " << bus_types[storageDeviceDescriptor->BusType] << endl;
	}
	if (storageDeviceDescriptor->SerialNumberOffset != 0)
	{
		cout << "Serial number:" << buffer+storageDeviceDescriptor->SerialNumberOffset << endl;
	}

	delete[] buffer;

}

void getMemoryDrive(DWORD driveNumber) {
	using namespace std;
	STORAGE_PROPERTY_QUERY storagePropertyQuery;

	string path;
	_ULARGE_INTEGER diskSpace;
	_ULARGE_INTEGER freeSpace;

	diskSpace.QuadPart = 0;
	freeSpace.QuadPart = 0;
	unsigned long int logicalDrivesCount = GetLogicalDrives();

	for (char volumeLetter = 'A'; volumeLetter < 'Z'; volumeLetter++) {
		if ((logicalDrivesCount >> volumeLetter - 65) & 1) {

			path = volumeLetter;
			path += ":";
			GetDiskFreeSpaceEx(path.c_str(), 0, &diskSpace, &freeSpace);
			diskSpace.QuadPart = diskSpace.QuadPart / (1024 * 1024*1024);
			freeSpace.QuadPart = freeSpace.QuadPart / (1024 * 1024*1024);

			path = "\\\\.\\";
			path += volumeLetter;
			path += ":";
			

			HANDLE volumeHandle = CreateFile(
				path.c_str(),
				0,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS,
				NULL
			);

			const DWORD bufferSize = 100;
			BYTE* buffer = new BYTE[bufferSize];
			ZeroMemory(buffer, bufferSize);

			DeviceIoControl(volumeHandle,
				IOCTL_STORAGE_GET_DEVICE_NUMBER,
				&storagePropertyQuery,
				sizeof(STORAGE_PROPERTY_QUERY),
				buffer,
				bufferSize,
				NULL,
				NULL);

			_STORAGE_DEVICE_NUMBER* deviceNumber = (_STORAGE_DEVICE_NUMBER*)buffer;

			if (deviceNumber->DeviceNumber == driveNumber) {

				if (diskSpace.QuadPart != 0)
				{
					cout << "Volume: " << volumeLetter << endl;
					cout << "Total space " << diskSpace.QuadPart << " Gb"<< endl;
					cout << "Free space " << freeSpace.QuadPart << " Gb" << endl;
					cout << "Busy space " << diskSpace.QuadPart - freeSpace.QuadPart << " Gb" << endl;
				}

			}

		}
	}

}


int main() {

	LPWSTR volumeName = new WCHAR[100];
	HANDLE hMemory= FindFirstVolumeW(volumeName, 100);

	for (int i = 0; ; ++i) {
		HANDLE h = getDriveHandle(i);
		if (h == INVALID_HANDLE_VALUE) break;
	
		getDriveInfo(h);

		getMemoryDrive(i);
		
		BOOL res = ReadSMARTInfo(i);
		ST_SMART_INFO * pSmart;

		for (int j = 0; j < MAX_ATTRIBUTES; ++j)
		{
			pSmart = GetSMARTValue(i, j);
			if (pSmart == NULL)continue;
			const char* smartInfo = SmartIndexToString(pSmart->m_ucAttribIndex);
			if (smartInfo != "") {
				printf("0x%02x %s: %u\n", pSmart->m_ucAttribIndex, smartInfo, (UINT)pSmart->m_ucValue);
			}
			
			

		}

		std::cout << std::setfill('-')<< std::setw(30)<<"\n";

	}

	system("pause");

}




