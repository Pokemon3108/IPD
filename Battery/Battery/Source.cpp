#include <iostream>
#include <Windows.h>
#include <string>

int getBatteryLevel(SYSTEM_POWER_STATUS status)
{
	return status.BatteryLifePercent;
}

std::string getBatteryChargeStatus(SYSTEM_POWER_STATUS status) {
	std::string info;
	int st = status.BatteryFlag;
	switch (st) {
	case 1:
		info = "High";
		break;
	case 2:
		info = "Low";
		break;
	case 4:
		info = "Critical";
		break;
	case 8:
		info = "Charging";
		break;
	case 128:
		info = "No system battery";
		break;
	case 255:
	default:
		info = "Unknown status";
		break;
	}
	return info;
}

std::string getACLineStatus(SYSTEM_POWER_STATUS status) {
	std::string info;
	switch (status.ACLineStatus) {
	case 0:
		info="Power off";
		break;
	case 1:
		info= "Power on";
		break;
	case 255:
	default:
		info= "Unknown";
		break;
	}
	return info;
}

void printAcWorkTime(SYSTEM_POWER_STATUS status) {
	long batteryFull = status.BatteryLifeTime;
	if (batteryFull != -1) {
		int seconds = batteryFull % 60;
		int totalminutes = batteryFull / 60;
		int minutes = totalminutes % 60;
		int hours = totalminutes / 60;
		std::cout << "Battery full time : "
			<< hours <<" h "<< minutes << " min " << seconds << " sec"<<std::endl;
	}
}

int main() {
	SYSTEM_POWER_STATUS status;
	GetSystemPowerStatus(&status);
	std::cout << "Current battery charge level: " << getBatteryLevel(status) <<"%"<< std::endl;
	std::cout << "Current battery status: " << getBatteryChargeStatus(status) << std::endl;
	std::cout << "Current battery charge status: " << getACLineStatus(status) << std::endl;
	printAcWorkTime(status);
	system("pause");
}