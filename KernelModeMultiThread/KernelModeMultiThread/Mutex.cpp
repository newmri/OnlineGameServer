#include<iostream>
#include<WinSock2.h>
#include<process.h>
using namespace std;

// 뮤텍스는 CRITICAL_SECTION과 비슷하나 커널 객체라서 다른 프로세스간의 쓰레드 동기화 가능, 기본적 자동 리셋 모드

int g_nInteger = 0;
HANDLE g_hMutex;

unsigned int __stdcall ThreadFunc(LPVOID lpVoid)
{
	
	while (true) {
		// g_hMutex객체가 Signaled 상태가 될 때 까지 대기
		WaitForSingleObject(g_hMutex, INFINITE);
		g_nInteger++;
		cout << "ThreadId: " << GetCurrentThreadId() << " :" << g_nInteger << endl;
		if (g_nInteger == 9 || g_nInteger == 10)
			break;
		ReleaseMutex(g_hMutex);
	}
	cout << "Thread end ThreadID: " << GetCurrentThreadId() << endl;
	ReleaseMutex(g_hMutex);
	return 0;
}

int main()
{
	unsigned int uiThreadID = 0;
	// 신호 상태 생성
	g_hMutex = CreateMutex(NULL, FALSE, NULL);
	if (NULL == g_hMutex) {
		cout << "CreateMutex() 함수 호출 실패:" << GetLastError() << endl;
	}

	// 쓰레드 생성
	HANDLE hThread1 = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, NULL, CREATE_SUSPENDED, &uiThreadID);
	cout << "Create Thread ID:" << uiThreadID << endl;
	HANDLE hThread2 = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, NULL, CREATE_SUSPENDED, &uiThreadID);
	cout << "Create Thread ID:" << uiThreadID << endl;
	if (NULL == hThread1 || NULL == hThread2) {
		cout << "[ERROR]Thread1 or Thread2 is NULL.." << endl;
		return 0;
	}
	ResumeThread(hThread1);
	ResumeThread(hThread2);
	// 소유권 해제
	ReleaseMutex(g_hMutex);

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	CloseHandle(g_hMutex);
	CloseHandle(hThread1);
	CloseHandle(hThread2);
	return 0;






}