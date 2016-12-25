#include<iostream>
#include<WinSock2.h>
#include<process.h>
using namespace std;

// ���ؽ��� CRITICAL_SECTION�� ����ϳ� Ŀ�� ��ü�� �ٸ� ���μ������� ������ ����ȭ ����, �⺻�� �ڵ� ���� ���

int g_nInteger = 0;
HANDLE g_hMutex;

unsigned int __stdcall ThreadFunc(LPVOID lpVoid)
{
	
	while (true) {
		// g_hMutex��ü�� Signaled ���°� �� �� ���� ���
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
	// ��ȣ ���� ����
	g_hMutex = CreateMutex(NULL, FALSE, NULL);
	if (NULL == g_hMutex) {
		cout << "CreateMutex() �Լ� ȣ�� ����:" << GetLastError() << endl;
	}

	// ������ ����
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
	// ������ ����
	ReleaseMutex(g_hMutex);

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	CloseHandle(g_hMutex);
	CloseHandle(hThread1);
	CloseHandle(hThread2);
	return 0;






}