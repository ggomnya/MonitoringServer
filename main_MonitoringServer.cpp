#include "CMonitoringLanServer.h"
#include "textparser.h"
#include <conio.h>
#include <time.h>



char LoginSessionKey[32];
WCHAR NetServerIP[16];
USHORT NetServerPort;
DWORD NetServerThreadNum;
DWORD NetServerIOCPNum;
DWORD NetServerMaxSession;
WCHAR LanServerIP[16];
USHORT LanServerPort;
DWORD LanServerThreadNum;
DWORD LanServerIOCPNum;
DWORD LanServerMaxSession;
WCHAR DBConnectIP[16];
WCHAR DBConnectID[16];
WCHAR DBConnectPw[16];

int wmain() {
	timeBeginPeriod(1);
	CINIParse Parse;
	Parse.LoadFile(L"MonitorServer_Config.ini");
	Parse.GetValue(L"IP", NetServerIP);
	Parse.GetValue(L"PORT", (DWORD*)&NetServerPort);
	Parse.GetValue(L"THREAD_NUMBER", &NetServerThreadNum);
	Parse.GetValue(L"IOCP_NUMBER", &NetServerIOCPNum);
	Parse.GetValue(L"MAX_SESSION", &NetServerMaxSession);

	Parse.GetValue(L"IP", LanServerIP);
	Parse.GetValue(L"PORT", (DWORD*)&LanServerPort);
	Parse.GetValue(L"THREAD_NUMBER", &LanServerThreadNum);
	Parse.GetValue(L"IOCP_NUMBER", &LanServerIOCPNum);
	Parse.GetValue(L"MAX_SESSION", &LanServerMaxSession);

	Parse.GetValue(L"DB_IP", DBConnectIP);
	Parse.GetValue(L"DB_ID", DBConnectID);
	Parse.GetValue(L"DB_PW", DBConnectPw);

	memcpy(LoginSessionKey, "ajfw@!cv980dSZ[fje#@fdj123948djf", 32);
	CMonitoringLanServer* server = new CMonitoringLanServer(LoginSessionKey, DBConnectIP, DBConnectID, DBConnectPw);
	//Net Server 세팅
	server->_MonitorNetServer->Start(INADDR_ANY, NetServerPort, NetServerThreadNum, NetServerIOCPNum, NetServerMaxSession);

	//Lan Server 세팅
	server->Start(INADDR_ANY, LanServerPort, LanServerThreadNum, LanServerIOCPNum, LanServerMaxSession);



	while (1) {
		int DataValue;
		int TimeStamp = time(NULL);
		server->TransferData(dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL, &DataValue, TimeStamp);
		wprintf(L"CPU Total: %d%%\n", DataValue);
		server->TransferData(dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY, &DataValue, TimeStamp);
		wprintf(L"Non Paged Total: %dMbytes\n", DataValue);
		server->TransferData(dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV, &DataValue, TimeStamp);
		wprintf(L"Network Recv: %dKbytes\n", DataValue);
		server->TransferData(dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND, &DataValue, TimeStamp);
		wprintf(L"Network Send: %dKbytes\n", DataValue);
		server->TransferData(dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY, &DataValue, TimeStamp);
		wprintf(L"Available Memory: %dMbytes\n\n", DataValue);

		if (_kbhit()) {
			WCHAR cmd = _getch();
			if (cmd == L'q' || cmd == L'Q')
				CCrashDump::Crash();
		}
		Sleep(999);
	}
}