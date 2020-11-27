#include "CMonitoringLanServer.h"
#include <Pdh.h>
#pragma comment(lib,"Pdh.lib")

CMonitoringLanServer::CMonitoringLanServer(char* LoginSessionKey, WCHAR* DBConnectIP, WCHAR* DBID, WCHAR* DBPW) {
	_MonitorNetServer = new CMonitoringNetServer(LoginSessionKey);
	//PDH 설정
	PdhOpenQuery(NULL, NULL, &_cpuQuery);
	PdhAddCounter(_cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &_cpuTotal);

	PdhOpenQuery(NULL, NULL, &_nonPagedQuery);
	PdhAddCounter(_nonPagedQuery, L"\\Memory\\Pool Nonpaged Bytes", NULL, &_nonPagedTotal);

	PdhOpenQuery(NULL, NULL, &_availableQuery);
	PdhAddCounter(_availableQuery, L"\\Memory\\Available MBytes", NULL, &_availableTotal);

	PdhOpenQuery(NULL, NULL, &_netRecvQuery[0]);
	PdhAddCounter(_netRecvQuery[0], L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes Received/sec", NULL, &_netRecvTotal[0]);
	PdhOpenQuery(NULL, NULL, &_netRecvQuery[1]);
	PdhAddCounter(_netRecvQuery[1], L"\\Network Interface(Realtek PCIe GBE Family Controller _2)\\Bytes Received/sec", NULL, &_netRecvTotal[1]);
	PdhOpenQuery(NULL, NULL, &_netRecvQuery[2]);
	PdhAddCounter(_netRecvQuery[2], L"\\Network Interface(Realtek PCIe GBE Family Controller _3)\\Bytes Received/sec", NULL, &_netRecvTotal[2]);


	PdhOpenQuery(NULL, NULL, &_netSendQuery[0]);
	PdhAddCounter(_netSendQuery[0], L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes Sent/sec", NULL, &_netSendTotal[0]);
	PdhOpenQuery(NULL, NULL, &_netSendQuery[1]);
	PdhAddCounter(_netSendQuery[1], L"\\Network Interface(Realtek PCIe GBE Family Controller _2)\\Bytes Sent/sec", NULL, &_netSendTotal[1]);
	PdhOpenQuery(NULL, NULL, &_netSendQuery[2]);
	PdhAddCounter(_netSendQuery[2], L"\\Network Interface(Realtek PCIe GBE Family Controller _3)\\Bytes Sent/sec", NULL, &_netSendTotal[2]);

	//DB 연결
	char chDBConnectIP[16];
	char chDBID[16];
	char chDBPW[16];
	WideCharToMultiByte(CP_UTF8, 0, DBConnectIP, 16, chDBConnectIP, 16, 0, 0);
	WideCharToMultiByte(CP_UTF8, 0, DBID, 16, chDBID, 16, 0, 0);
	WideCharToMultiByte(CP_UTF8, 0, DBPW, 16, chDBPW, 16, 0, 0);
	_DBConnector_TLS = new CDBConnector_TLS(chDBConnectIP, chDBID, chDBPW);
	_DBConnector_TLS->Connect();
	for (int i = 0; i < 45; i++) {
		if (i <= 6) {
			_dwMonitoringInfo[i][eSERVERNO] = 1;
		}
		else if (i <= 23 && i >= 10) {
			_dwMonitoringInfo[i][eSERVERNO] = 2;
		}
		else if (i >= 30 && i <= 37) {
			_dwMonitoringInfo[i][eSERVERNO] = 3;
		}
		else if (i >= 40 && i <= 44) {
			_dwMonitoringInfo[i][eSERVERNO] = 4;
		}
		else
			_dwMonitoringInfo[i][eSERVERNO] = 0;
		_dwMonitoringInfo[i][eMIN] = -1;
		_dwMonitoringInfo[i][eMAX] = -1;
		_dwMonitoringInfo[i][eAVG] = -1;
		_dwMonitoringInfo[i][eCOUNT] = 0;
		_dwMonitoringInfo[i][eTIME] = 0;
	}
}

void CMonitoringLanServer::MPMonitorToolDataUpdate(CPacket* pPacket, WORD Type, BYTE ServerNo, BYTE DataType, int DataValue, int TimeStamp) {
	*pPacket << Type << ServerNo << DataType << DataValue << TimeStamp;
}

void CMonitoringLanServer::TransferData(BYTE DataType, int *Data, int TimeStamp) {
	BYTE ServerNo = 4;
	int DataValue;
	PDH_FMT_COUNTERVALUE counterVal;
	switch (DataType) {
	case dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL:
	{
		PdhCollectQueryData(_cpuQuery);
		PdhGetFormattedCounterValue(_cpuTotal, PDH_FMT_LONG, NULL, &counterVal);
		DataValue = counterVal.longValue;
	}
	break;
	case dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY:
	{
		PdhCollectQueryData(_nonPagedQuery);
		PdhGetFormattedCounterValue(_nonPagedTotal, PDH_FMT_LARGE, NULL, &counterVal);
		DataValue = (counterVal.largeValue / (1000 * 1000));
	}
	break;
	case dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV:
	{
		PdhCollectQueryData(_netRecvQuery[0]);
		PdhGetFormattedCounterValue(_netRecvTotal[0], PDH_FMT_LARGE, NULL, &counterVal);
		INT64 NetRecv = counterVal.largeValue;
		PdhCollectQueryData(_netRecvQuery[1]);
		PdhGetFormattedCounterValue(_netRecvTotal[1], PDH_FMT_LARGE, NULL, &counterVal);
		NetRecv += counterVal.largeValue;
		PdhCollectQueryData(_netRecvQuery[2]);
		PdhGetFormattedCounterValue(_netRecvTotal[2], PDH_FMT_LARGE, NULL, &counterVal);
		NetRecv += counterVal.largeValue;
		DataValue = (NetRecv / 1000);
	}
	break;
	case dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND:
	{
		PdhCollectQueryData(_netSendQuery[0]);
		PdhGetFormattedCounterValue(_netSendTotal[0], PDH_FMT_LARGE, NULL, &counterVal);
		INT64 NetSend = counterVal.largeValue;
		PdhCollectQueryData(_netSendQuery[1]);
		PdhGetFormattedCounterValue(_netSendTotal[1], PDH_FMT_LARGE, NULL, &counterVal);
		NetSend += counterVal.largeValue;
		PdhCollectQueryData(_netSendQuery[2]);
		PdhGetFormattedCounterValue(_netSendTotal[2], PDH_FMT_LARGE, NULL, &counterVal);
		NetSend += counterVal.largeValue;
		DataValue = (NetSend / 1000);
	}
	break;
	case dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY:
	{
		PdhCollectQueryData(_availableQuery);
		PdhGetFormattedCounterValue(_availableTotal, PDH_FMT_LONG, NULL, &counterVal);
		DataValue = counterVal.longValue;
	}
	break;
	default:
		CCrashDump::Crash();
		break;
	}
	*Data = DataValue;
	CPacket* pSendPacket = CPacket::Alloc();
	MPMonitorToolDataUpdate(pSendPacket, en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE, ServerNo, DataType, DataValue, TimeStamp);
	_MonitorNetServer->SendPacket_Broad(pSendPacket);
	pSendPacket->Free();
	SettingData(DataType, DataValue, TimeStamp);
}

void CMonitoringLanServer::SettingData(BYTE DataType, int DataValue, int TimeStamp) {
	//최소값 세팅
	if (_dwMonitoringInfo[DataType][eMIN] == -1) {
		_dwMonitoringInfo[DataType][eMIN] = DataValue;
	}
	else {
		if (_dwMonitoringInfo[DataType][eMIN] > DataValue)
			_dwMonitoringInfo[DataType][eMIN] = DataValue;
	}
	//최대값 세팅
	if (_dwMonitoringInfo[DataType][eMAX] == -1) {
		_dwMonitoringInfo[DataType][eMAX] = DataValue;
	}
	else {
		if (_dwMonitoringInfo[DataType][eMAX] < DataValue)
			_dwMonitoringInfo[DataType][eMAX] = DataValue;
	}

	//평균값 세팅
	if (_dwMonitoringInfo[DataType][eAVG] == -1) {
		_dwMonitoringInfo[DataType][eAVG] = DataValue;
	}
	else {
		_dwMonitoringInfo[DataType][eAVG] += DataValue;
	}

	_dwMonitoringInfo[DataType][eCOUNT]++;
	
	if (_dwMonitoringInfo[DataType][eTIME] == 0) {
		_dwMonitoringInfo[DataType][eTIME] = time(NULL);
	}
	//데이터를 전송한 지 1분이 경과한 경우
	if (time(NULL) - _dwMonitoringInfo[DataType][eTIME] >= 60) {
		//DB 저장하기
		SaveDB(DataType);
		_dwMonitoringInfo[DataType][eMIN] = -1;
		_dwMonitoringInfo[DataType][eMAX] = -1;
		_dwMonitoringInfo[DataType][eAVG] = -1;
		_dwMonitoringInfo[DataType][eCOUNT] = 0;
		_dwMonitoringInfo[DataType][eTIME] = 0;
	}
}

void CMonitoringLanServer::SaveDB(BYTE DataType) {
	tm TM;
	time_t t;
	time(&t);
	localtime_s(&TM, &t);
	char tableName[256];
	StringCchPrintfA(tableName, 256, "log_%04d%02d", TM.tm_year + 1900, TM.tm_mon + 1);
	char query[1000];
	sprintf_s(query, "INSERT INTO %s (serverNo, time, datatype, avg, min, max, count) VALUES (%d, %d, %d, %d, %d, %d, %d)", 
		tableName, _dwMonitoringInfo[DataType][eSERVERNO], _dwMonitoringInfo[DataType][eTIME], DataType, _dwMonitoringInfo[DataType][eAVG] / _dwMonitoringInfo[DataType][eCOUNT],
		_dwMonitoringInfo[DataType][eMIN], _dwMonitoringInfo[DataType][eMAX], _dwMonitoringInfo[DataType][eCOUNT]);
	bool retval = _DBConnector_TLS->Query(query);
	if (!retval) {
		//table이 없는 것
		if (_DBConnector_TLS->Error() == 1146) {
			char query2[1000];
			sprintf_s(query2, "CREATE TABLE `%s` LIKE `log_template`", tableName);
			retval = _DBConnector_TLS->Query(query2);
			if (!retval)
				CCrashDump::Crash();
			else
				_DBConnector_TLS->Query(query);
		}
		else
			CCrashDump::Crash();
	}
}

void CMonitoringLanServer::OnClientJoin(SOCKADDR_IN clientaddr, INT64 SessionID) {
	return;
}
void CMonitoringLanServer::OnClientLeave(INT64 SessionID) {
	return;
}

bool CMonitoringLanServer::OnConnectRequest(SOCKADDR_IN clientaddr) {
	return true;
}

void CMonitoringLanServer::OnRecv(INT64 SessionID, CPacket* pRecvPacket) {
	WORD Type;
	int ServerNo;
	*pRecvPacket >> Type;
	//서버로 로그인 한 경우
	if (Type == en_PACKET_SS_MONITOR_LOGIN) {
		*pRecvPacket >> ServerNo;
		for (int i = 0; i < 45; i++) {
			if (_dwMonitoringInfo[i][eSERVERNO] == ServerNo) {
				_dwMonitoringInfo[i][eMIN] = -1;
				_dwMonitoringInfo[i][eMAX] = -1;
				_dwMonitoringInfo[i][eCOUNT] = 0;
				_dwMonitoringInfo[i][eTIME] = 0;
				_dwMonitoringInfo[i][eAVG] = -1;
			}
		}
	}
	else if (Type == en_PACKET_SS_MONITOR_DATA_UPDATE) {
		BYTE DataType;
		int DataValue;
		int TimeStamp;
		BYTE ServerNo;
		*pRecvPacket >> DataType >> DataValue >> TimeStamp;
		ServerNo = _dwMonitoringInfo[DataType][eSERVERNO];
		//모니터링 클라에 데이터 전송하기
		CPacket* pSendPacket = CPacket::Alloc();
		MPMonitorToolDataUpdate(pSendPacket, en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE, ServerNo, DataType, DataValue, TimeStamp);
		_MonitorNetServer->SendPacket_Broad(pSendPacket);
		pSendPacket->Free();
		SettingData(DataType, DataValue, TimeStamp);

	}
	else {
		Disconnect(SessionID);
	}
}

void CMonitoringLanServer::OnError(int errorcode, const WCHAR* Err) {
	return;
}

CMonitoringLanServer::~CMonitoringLanServer() {
	delete _DBConnector_TLS;
}