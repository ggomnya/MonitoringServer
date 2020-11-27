#pragma once
#include "CLanServer.h"
#include "MonitorProtocol.h"
#include "CMonitoringNetServer.h"
#include "CDBConnector.h"
#include "CDBConnector_TLS.h"
#include <time.h>
#include <strsafe.h>
#include <Pdh.h>
#pragma comment(lib,"Pdh.lib")
/*
	ServerNo: 1 - Login, 2 - Game, 3 - Chat, 4 - Monitor

*/
class CMonitoringLanServer : public CLanServer {
	// PDH 쿼리 핸들 생성
	PDH_HQUERY _cpuQuery;
	PDH_HCOUNTER _cpuTotal;

	PDH_HQUERY _nonPagedQuery;
	PDH_HCOUNTER _nonPagedTotal;

	PDH_HQUERY _availableQuery;
	PDH_HCOUNTER _availableTotal;

	PDH_HQUERY _netRecvQuery[3];
	PDH_HCOUNTER _netRecvTotal[3];

	PDH_HQUERY _netSendQuery[3];
	PDH_HCOUNTER _netSendTotal[3];

public:
	CDBConnector_TLS* _DBConnector_TLS;
	CMonitoringNetServer* _MonitorNetServer;
	CMonitoringLanServer(char* LoginSessionKey, WCHAR* DBConnectIP, WCHAR* DBID, WCHAR* DBPW);
	DWORD _dwMonitoringInfo[45][6];
	~CMonitoringLanServer();

	void TransferData(BYTE DataType, int* Data, int TimeStamp);
	void MPMonitorToolDataUpdate(CPacket* pPacket, WORD Type, BYTE ServerNo, BYTE DataType, int DataValue, int TimeStamp);
	void SettingData(BYTE DataType, int DataValue, int TimeStamp);
	void SaveDB(BYTE DataType);

	virtual void OnClientJoin(SOCKADDR_IN clientaddr, INT64 SessionID);
	virtual void OnClientLeave(INT64 SessionID);

	virtual bool OnConnectRequest(SOCKADDR_IN clientaddr);

	virtual void OnRecv(INT64 SessionID, CPacket* pRecvPacket);
	//virtual void OnSend(INT64 SessionID, int SendSize) = 0;

	virtual void OnError(int errorcode, const WCHAR* Err); 

};