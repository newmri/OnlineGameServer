#include<iostream>
#include<WinSock2.h>
#include<process.h>
using namespace std;

// 세마포어는 뮤텍스와 달리 카운트를 두어 동시 실행되는 쓰레드의 수를 설정 가능, 자동 리셋
// 바이너리 세마포어: 쓰레드 수를 한개로 제한, 뮤텍스와 동일 기능 수행
// 일반적인 세마포어는 쓰레드가 동시에 같은 공유 자원 접근 가능, 크리티컬 섹션이나 뮤텍스 같은 다른 동기화와 함께 사용



int g_nInteger = 0;
HANDLE g_hSemaphore;

unsigned int __stdcall ThreadFunc(LPVOID lpVoid)
{
	LONG lPre = 0;
	while (true) {
		// g_hSemaphore객체가 신호 받음 상태가 될 때까지 대기, 실행 상태가 되면 내부 카운트를 1 감소
		WaitForSingleObject(g_hSemaphore, INFINITE);
		g_nInteger++;
		cout << "ThreadId: " << GetCurrentThreadId() << " :" << g_nInteger << endl;
		if (g_nInteger >= 8){
			cout << "ThreadId: " << GetCurrentThreadId() << " 종료:" << g_nInteger << endl;
			break;
		}
		// 소유권을 해제, 내부 카운트 1증가, 객체의 상태를 신호 받음 상태로 바꿔줌
		ReleaseSemaphore(g_hSemaphore, 1, &lPre);
	}
	cout << "Thread end ThreadID: " << GetCurrentThreadId() << endl;
	ReleaseSemaphore(g_hSemaphore, 1, &lPre);
	return 0;
}

int main()
{
	unsigned int uiThreadID = 0;
	// 비 신호 상태 생성
	g_hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (NULL == g_hSemaphore) {
		cout << "CreateSemaphore() 함수 호출 실패:" << GetLastError() << endl;
	}

	// 쓰레드 생성
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
	// 소유권 해제
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