#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm.lib")

#include <process.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <stdio.h>
#include <locale.h>
#include "PacketBuffer.h"
#include "LockfreeQueue.h"
#include "LockfreeStack.h"
#include "RingBuffer_Lock.h"
#include "CCrashDump.h"
#include "CommonStruct.h"
using namespace std;




class CLanServer {
private:
	stSESSION* _SessionList;
	CLockfreeStack<int> _IndexSession;
	INT64 _SessionIDCnt;
	SOCKET _Listen_sock;
	HANDLE _hcp;
	HANDLE* _hWorkerThread;
	HANDLE _hAcceptThread;

	int _MaxSession;
	LONG _SessionCnt;
	int _NumThread;
public:
	int _AcceptCount;
	int _AcceptTPS;
	volatile LONG _SendTPS;
	volatile LONG _RecvTPS;
	int _DisCount;

	CLanServer();
	static unsigned int WINAPI WorkerThreadFunc(LPVOID lParam) {
		((CLanServer*)lParam)->WorkerThread(lParam);
		return 0;
	}
	static unsigned int WINAPI AcceptThreadFunc(LPVOID lParam) {
		((CLanServer*)lParam)->AcceptThread(lParam);
		return 0;
	}
	unsigned int WINAPI WorkerThread(LPVOID lParam);

	unsigned int WINAPI AcceptThread(LPVOID lParam);

	bool Start(ULONG OpenIP, USHORT Port, int NumWorkerthread, int NumIOCP, int MaxSession, 
		int iBlockNum = 100, bool bPlacementNew = false, bool Restart = false);
	
	void Stop();
	int GetClientCount();
	bool _Disconnect(stSESSION* pSession);
	bool Disconnect(INT64 SessionID);
	bool _SendPacket(stSESSION* pSession, CPacket* pSendPacket, int type = eLAN);
	bool SendPacket(INT64 SessionID, CPacket* pSendPacket, int type = eLAN);
	stSESSION* FindSession(INT64 SessionID);
	void ReleaseSession(stSESSION* pSession);

	void RecvPost(stSESSION* pSession);
	void SendPost(stSESSION* pSession);
	void Release(stSESSION* pSession);

	//void DebugFunc(stSESSION* pSession, int FuncNum);


	

	virtual void OnClientJoin(SOCKADDR_IN clientaddr, INT64 SessionID) = 0;
	virtual void OnClientLeave(INT64 SessionID) = 0;

	virtual bool OnConnectRequest(SOCKADDR_IN clientaddr) = 0;

	virtual void OnRecv(INT64 SessionID, CPacket* pRecvPacket) = 0;
	//virtual void OnSend(INT64 SessionID, int SendSize) = 0;

	virtual void OnError(int errorcode, const WCHAR* Err) = 0;

};