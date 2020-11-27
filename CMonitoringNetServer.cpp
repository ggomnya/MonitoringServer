#include "CMonitoringNetServer.h"

CMonitoringNetServer::CMonitoringNetServer(char* LoginSessionKey) {
	InitializeCriticalSection(&_csCLIENT);
	memcpy(_LoginSessionKey, LoginSessionKey, 32);
}

void CMonitoringNetServer::InsertClientSet(INT64 SessionID) {
	EnterCriticalSection(&_csCLIENT);
	_clientSet.insert(SessionID);
	LeaveCriticalSection(&_csCLIENT);

}
INT64 CMonitoringNetServer::FindClientSet(INT64 SessionID) {
	EnterCriticalSection(&_csCLIENT);
	auto it = _clientSet.find(SessionID);
	if (it != _clientSet.end()) {
		LeaveCriticalSection(&_csCLIENT);
		return SessionID;
	}
	else {
		LeaveCriticalSection(&_csCLIENT);
		return -1;
	}

}
void CMonitoringNetServer::RemoveClientSet(INT64 SessionID) {
	EnterCriticalSection(&_csCLIENT);
	_clientSet.erase(SessionID);
	LeaveCriticalSection(&_csCLIENT);
}

void CMonitoringNetServer::MPMonitorToolResLogin(CPacket* pPacket, WORD Type, BYTE Status) {
	*pPacket << Type << Status;
}

void CMonitoringNetServer::SendPacket_Broad(CPacket* pSendPacket) {
	EnterCriticalSection(&_csCLIENT);
	for (auto it = _clientSet.begin(); it != _clientSet.end(); it++) {
		SendPacket(*it, pSendPacket);
	}
	LeaveCriticalSection(&_csCLIENT);
}

void CMonitoringNetServer::OnClientJoin(SOCKADDR_IN clientaddr, INT64 SessionID) {
	InsertClientSet(SessionID);
	return;
}
void CMonitoringNetServer::OnClientLeave(INT64 SessionID) {
	RemoveClientSet(SessionID);
	return;
}

bool CMonitoringNetServer::OnConnectRequest(SOCKADDR_IN clientaddr) {
	return true;
}

void CMonitoringNetServer::OnRecv(INT64 SessionID, CPacket* pRecvPacket) {
	WORD Type;
	char Token[32];
	*pRecvPacket >> Type;
	if (Type != en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN) {
		Disconnect(SessionID);
	}
	else {
		pRecvPacket->GetData(Token, 32);
		//token이 일치하지 않는 경우
		if (memcmp(_LoginSessionKey, Token, 32) != 0) {
			Disconnect(SessionID);
		}
		else {
			CPacket* pSendPacket = CPacket::Alloc();
			MPMonitorToolResLogin(pSendPacket, en_PACKET_CS_MONITOR_TOOL_RES_LOGIN, dfMONITOR_TOOL_LOGIN_OK);
			SendPacket_Broad(pSendPacket);
		}
	}
}

void CMonitoringNetServer::OnError(int errorcode, const WCHAR* Err) {
	return;
}