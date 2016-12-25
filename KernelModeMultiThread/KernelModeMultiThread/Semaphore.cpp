#include<iostream>
#include<WinSock2.h>
#include<process.h>
using namespace std;

// ��������� ���ؽ��� �޸� ī��Ʈ�� �ξ� ���� ����Ǵ� �������� ���� ���� ����, �ڵ� ����
// ���̳ʸ� ��������: ������ ���� �Ѱ��� ����, ���ؽ��� ���� ��� ����
// �Ϲ����� ��������� �����尡 ���ÿ� ���� ���� �ڿ� ���� ����, ũ��Ƽ�� �����̳� ���ؽ� ���� �ٸ� ����ȭ�� �Բ� ���



int g_nInteger = 0;
HANDLE g_hSemaphore;

unsigned int __stdcall ThreadFunc(LPVOID lpVoid)
{
	LONG lPre = 0;
	while (true) {
		// g_hSemaphore��ü�� ��ȣ ���� ���°� �� ������ ���, ���� ���°� �Ǹ� ���� ī��Ʈ�� 1 ����
		WaitForSingleObject(g_hSemaphore, INFINITE);
		g_nInteger++;
		cout << "ThreadId: " << GetCurrentThreadId() << " :" << g_nInteger << endl;
		if (g_nInteger >= 8){
			cout << "ThreadId: " << GetCurrentThreadId() << " ����:" << g_nInteger << endl;
			break;
		}
		// �������� ����, ���� ī��Ʈ 1����, ��ü�� ���¸� ��ȣ ���� ���·� �ٲ���
		ReleaseSemaphore(g_hSemaphore, 1, &lPre);
	}
	cout << "Thread end ThreadID: " << GetCurrentThreadId() << endl;
	ReleaseSemaphore(g_hSemaphore, 1, &lPre);
	return 0;
}

int main()
{
	unsigned int uiThreadID = 0;
	// �� ��ȣ ���� ����
	g_hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (NULL == g_hSemaphore) {
		cout << "CreateSemaphore() �Լ� ȣ�� ����:" << GetLastError() << endl;
	}

	// ������ ����
	HANDLE hThread1 = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, NULL, CREATE_SUSPENDED, &uiThreadID);
	cout << "Create Thread ID:" << uiThreadID << endl;
	HANDLE hThread2 = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, NULL, CREATE_SUSPENDED, &uiThreadID);
	cout << "Create Thread ID:" << uiThreadID << endl;
	HANDLE hThread3 = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, NULL, CREATE_SUSPENDED, &uiThreadID);
	cout << "Create Thread ID:" << uiThreadID << endl;

	if (NULL == hThread1 || NULL == hThread2 || NULL == hThread3) {
		cout << "[ERROR]Thread1 or Thread2 or Thread3 is NULL.." << endl;
		return 0;
	}
	ResumeThread(hThread1);
	ResumeThread(hThread2);
	ResumeThread(hThread3);
	// ������ ����
	LONG lPre = 0;
	ReleaseSemaphore(g_hSemaphore, 1, &lPre);

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);
	WaitForSingleObject(hThread3, INFINITE);

	CloseHandle(g_hSemaphore);
	CloseHandle(hThread1);
	CloseHandle(hThread2);
	CloseHandle(hThread3);
	return 0;






}