#include<iostream>
#include<WinSock2.h>
#include<process.h>
using namespace std;

int g_nInteger = 0;
CRITICAL_SECTION csInteger;


unsigned int __stdcall ThreadFunc(LPVOID lpVoid)
{
	while (true)
	{
		EnterCriticalSection(&csInteger);
		g_nInteger++;
		cout << g_nInteger << endl;
		if (10 == g_nInteger || 9 == g_nInteger)
			break;
		LeaveCriticalSection(&csInteger);
	}
	LeaveCriticalSection(&csInteger);
	cout << "ThreadFunc end..ThreadID:" << GetCurrentThreadId() << endl;
	return 0;
}


int main()
{

	unsigned int uiThreadID = 0;
	InitializeCriticalSection(&csInteger);

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
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	CloseHandle(hThread1);
	CloseHandle(hThread2);
	return 0;
}

