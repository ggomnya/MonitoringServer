/********
	2020.02.05 (��)

	�ؽ�Ʈ �ļ� 

	GetValue : �����ε��ϱ�

	���� ���� ��Ȳ �� ó���� �� �ֵ��� �ϱ�

	�����ڵ� ��Ȳ������ ó���� �� �ְ� ¥����

********/

//#define UNICODE
//#define _UNICODE

//#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <strsafe.h>

class CINIParse {
private:
	WCHAR* _FileInfo;
	DWORD _FileSize;
	WCHAR* _chpBuffer;
public:
	CINIParse() {
		_FileInfo = NULL;
		_chpBuffer = NULL;
		_FileSize = 0;
	}
	~CINIParse() {
		if (_FileInfo != NULL) {
			free(_FileInfo);
		}
	}
	BOOL LoadFile (LPCTSTR fileName) {
		FILE* fp;
		DWORD err = _wfopen_s(&fp, fileName, L"rb");
		if (err == 0) {
			fseek(fp, 0, SEEK_END);
			_FileSize = ftell(fp);
			_FileInfo = (WCHAR*)malloc(sizeof(WCHAR) * _FileSize);
 			fseek(fp, 0, SEEK_SET);
			fread(_FileInfo, sizeof(WCHAR), _FileSize, fp);
			fclose(fp);
			_chpBuffer = _FileInfo;
			return TRUE;
		}
		else return FALSE;
	}

	BOOL isEnd() {
		if (_chpBuffer - _FileInfo >= _FileSize)
			return TRUE;
		else return FALSE;
	}

	BOOL SkipNonCommand() {
		while (1) {
			if (*_chpBuffer == L',' || *_chpBuffer == L'"' || *_chpBuffer == 0x20 ||
				*_chpBuffer == 0x08 || *_chpBuffer == 0x09 || *_chpBuffer == 0x0a ||
				*_chpBuffer == 0x0d) {

			}
			//�ּ��� ���
			else if (*_chpBuffer == L'/' && *(_chpBuffer+1) == L'/') {
				while (1) {
					if (*_chpBuffer == 0x0d && *(_chpBuffer + 1) == 0x0a) {
						break;
					}
					_chpBuffer++;
				}
				_chpBuffer++;
			}
			/*�ּ��� ���*/
			else if (*_chpBuffer == '/' && *(_chpBuffer + 1) == '*') {
				while (1) {
					if (*_chpBuffer == '*' && *(_chpBuffer + 1) == '/') {
						break;
					}
					_chpBuffer++;
				}
				_chpBuffer++;
			}
			else {
				return !isEnd();
			}			
			_chpBuffer++;

		}
	}

	BOOL GetNextWord(WCHAR** Buffer, DWORD* ipLength) {
		//�ܾ ������ ���
		if (SkipNonCommand()) {
			DWORD TempLength = 0;
			*Buffer = _chpBuffer;
			while (!isEnd()) {
				if (*_chpBuffer == L',' || *_chpBuffer == L'"' || *_chpBuffer == 0x20 ||
					*_chpBuffer == 0x08 || *_chpBuffer == 0x09 || *_chpBuffer == 0x0a ||
					*_chpBuffer == 0x0d) {
					*ipLength = TempLength;
					break;
				}
				_chpBuffer++;
				TempLength++;
			}
			return !isEnd();
		}
		else {
			return FALSE;
		}
	}

	BOOL GetStringWord(WCHAR** Buffer, DWORD* ipLength) {
		if (SkipNonCommand()) {
			DWORD TempLength = 0;
			*Buffer = _chpBuffer;

			//���ڿ��� �ƴ� ��� pass
			if (*(_chpBuffer - 1) != L'"')
				return FALSE;

			while (!isEnd()) {
				if (*_chpBuffer == L'"') {
					if (*(_chpBuffer-1) != 0x5c) {
						//TempLength--;
						*ipLength = TempLength;
						break;
					}
				}
				_chpBuffer++;
				TempLength++;
			}
			return !isEnd();
		}
		else {
			return FALSE;
		}
	}

	//int�� value�� ��� �Լ�
	BOOL GetValue(LPCTSTR szName, DWORD* ipValue) {
		WCHAR chWord[256];
		WCHAR* Buffer;
		DWORD iLength;
		while (GetNextWord(&Buffer, &iLength)) {
			memset(chWord, 0, 256);
			memcpy_s(chWord, 256, Buffer, iLength*2);

			//�Է¹��� �ܾ�� ������ �˻�
			if (wcscmp(szName, chWord)==0) {
				// �´ٸ� = ã��
				if (GetNextWord(&Buffer, &iLength)) {
					memset(chWord, 0, 256);
					memcpy_s(chWord, 256, Buffer, iLength*2);
					if (wcscmp(chWord, L"=") == 0) {
						// = �ڿ� ������ Ȯ��
						if (GetNextWord(&Buffer, &iLength)) {
							memset(chWord, 0, 256);
							memcpy_s(chWord, 256, Buffer, iLength*2);
							*ipValue = _wtoi(chWord);
							//_chpBuffer = _FileInfo;
							//���ڿ��� �߸� ���� ��� 
							if (*(Buffer - 1) == '"')
								return FALSE;
							//���� �˻����� �ٽ� ó������ �˻��ؾ� �ϹǷ� ���� ù�κ��� ����Ű���� ��
							return TRUE;
						}
					}
				}
			}
		}
		//���� �˻����� �ٽ� ó������ �˻��ؾ� �ϹǷ� ���� ù�κ��� ����Ű���� ��
		_chpBuffer = _FileInfo;
		return FALSE;
	}

	//���ڿ� value�� ��� �Լ�
	BOOL GetValue(LPCTSTR szName, TCHAR* ipValue) {
		TCHAR chWord[256];
		TCHAR* Buffer;
		DWORD iLength;
		while (GetNextWord(&Buffer, &iLength)) {
			memset(chWord, 0, 256);
			memcpy_s(chWord, 256, Buffer, iLength*2);

			//�Է¹��� �ܾ�� ������ �˻�
			if (wcscmp(szName, chWord) == 0) {
				// �´ٸ� = ã��
				if (GetNextWord(&Buffer, &iLength)) {
					memset(chWord, 0, 256);
					memcpy_s(chWord, 256, Buffer, iLength*2);
					if (wcscmp(chWord, L"=") == 0) {
						// = �ڿ� ������ Ȯ��
						if (GetStringWord(&Buffer, &iLength)) {
							memset(chWord, 0, 256);
							//\�� �޸𸮿��� ������ ��
							DWORD TempLength = 0;
							for (DWORD i = 0; i < iLength; i++) {
								if (*(Buffer + i) != L'\\') {
									chWord[TempLength] = *(Buffer + i);
									TempLength++;
								}
								else {
									if (i + 1 == iLength)
										continue;
									if (*(Buffer + i+1) == L'\\') {
										chWord[TempLength] = *(Buffer + i);
										TempLength++;
									}
								}
							}
							StringCchCopyN(ipValue, 256, chWord, 256);
							//_chpBuffer = _FileInfo;
							//���� �˻����� �ٽ� ó������ �˻��ؾ� �ϹǷ� ���� ù�κ��� ����Ű���� ��
							return TRUE;
						}
					}
				}
			}
		}
		//���� �˻����� �ٽ� ó������ �˻��ؾ� �ϹǷ� ���� ù�κ��� ����Ű���� ��
		_chpBuffer = _FileInfo;
		return FALSE;
	}

	void PrintFile() {
		wprintf(L"%s", _FileInfo);
	}

};

//int wmain(DWORD argv, TCHAR* argc) {
//	CINIParse Parse;
//	Parse.LoadFile(L"Config1.txt");
//	DWORD len = 0;
//	//Parse.GetValue(_T("������"), &len);
//	//NET_SERVER
//	WCHAR ServerIP[16];
//	USHORT ServerPort;
//	Parse.GetValue(L"IP", ServerIP);
//	Parse.GetValue(L"PORT", (DWORD*)&ServerPort);
//	wprintf(L"%s\n", ServerIP);
//	wprintf(L"%d\n", ServerPort);
//
//}
