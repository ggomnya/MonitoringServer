#pragma once
#include "CNetServer.h"
#include "MonitorProtocol.h"
#include <unordered_set>

class CMonitoringNetServer : public CNetServer {
public:
	CRITICAL_SECTION _csCLIENT;
	unordered_set<INT64> _clientSet;
	char _LoginSessionKey[64];
	CMonitoringNetServer(char* LoginSessionKey);

	void InsertClientSet(INT64 SessionID);
	INT64 FindClientSet(INT64 SessionID);
	void RemoveClientSet(INT64 SessionID);

	void MPMonitorToolResLogin(CPacket* pPacket, WORD Type, BYTE Status);
	void SendPacket_Broad(CPacket* pSendPacket);

	virtual void OnClientJoin(SOCKADDR_IN clientaddr, INT64 SessionID);
	virtual void OnClientLeave(INT64 SessionID);

	virtual bool OnConnectRequest(SOCKADDR_IN clientaddr);

	virtual void OnRecv(INT64 SessionID, CPacket* pRecvPacket);
	//virtual void OnSend(INT64 SessionID, int SendSize) = 0;

	virtual void OnError(int errorcode, const WCHAR* Err);
};