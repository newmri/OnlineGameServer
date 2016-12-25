#include<iostream>
#include<Windows.h>
#include<process.h>
using namespace std;

int g_nInteger = 0;
HANDLE g_hEvent;
unsigned int __stdcall ThreadFunc(void* lpVoid)
{
	while (true) {
		// g_hEvent객체가 Signaled 상태가 될 때 까지 대기
		WaitForSingleObject(g_hEvent, INFINITE);
		g_nInteger++;
		cout << "ThreadId: " << GetCurrentThreadId() << " :" << g_nInteger << endl;
		if (g_nInteger == 9 || g_nInteger == 10)
			break;
		SetEvent(g_hEvent);
	}
	cout << "Thread end ThreadID: " << GetCurrentThreadId() << endl;
	SetEvent(g_hEvent);
	return 0;
}

int main(int argc, char* argv[])
{
	unsigned int uiThreadID = 0;
	g_hEvent = CreateEvent(NULL,FALSE,FALSE, NULL);
	if (NULL == g_hEvent) {
		cout << "CreateEvent() 함수 호출 실패:" << GetLastError() << endl;
	}

	// 쓰레드 생성
	HANDLE hThread1 = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, NULL,CREATE_SUSPENDED, &uiThreadID);
	cout << "Create Thread ID:" << uiThreadID << endl;
	HANDLE hThread2 = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, NULL, CREATE_SUSPENDED, &uiThreadID);
	cout << "Create Thread ID:" << uiThreadID << endl;
	if (NULL == hThread1 || NULL == hThread2) {
		cout << "[ERROR] Thread1 or hThread2 is NULL.." << endl;
		return 0;
	}
	ResumeThread(hThread1);
	ResumeThread(hThread2);
	SetEvent(g_hEvent);
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);
	CloseHandle(g_hEvent);
	CloseHandle(hThread1);
	CloseHandle(hThread2);
	return 0;






}