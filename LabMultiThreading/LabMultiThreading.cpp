// LabMultiThreading.cpp: entry point.
//

#include "stdafx.h"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <chrono>
#include <queue>
	
CRITICAL_SECTION cs;

//using namespace std;
std::ifstream g_inputStream;
std::ofstream g_outputStream;

int g_threadCount;
int g_minNumber = 2, g_maxNumber = 100;

//std::vector<int> g_possibleCurrentNumbers;

int g_countOfCombinations = 0;
int g_targetNumber = 0;
std::string g_millisecondsSpentOutputStr;

namespace SimpleTimer {
	class SimpleTimer
	{
	public:

		SimpleTimer::SimpleTimer()
		{
			start = std::chrono::high_resolution_clock::now();
		}

		std::string SimpleTimer::SetStart() {
			start = std::chrono::high_resolution_clock::now();
		}

		std::string SimpleTimer::Stop()
		{
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
			std::string strResult;

			strResult.append(std::to_string(millis));
			return strResult;
		}

		SimpleTimer::~SimpleTimer()
		{
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			float result = duration.count();
			std::cout << "Time spent: " << result << " seconds" << std::endl;
		}
	private:
		std::chrono::time_point<std::chrono::steady_clock> start, end;
		std::chrono::duration<float> duration;
	};
}

//SimpleTimer::SimpleTimer g_simpleTimer;

namespace CombinationsFinder {
	bool showResults = true;
	/*    arr - array to store the combination
	index - next location in array
	num - given number
	reducedNum - reduced number */
	void findCombinationsUtil(int arr[], int index,
		int num, int reducedNum)
	{
		// Base condition 
		if (reducedNum < 0)
			return;

		// If combination is found, print it 
		if (reducedNum == 0)
		{
			g_countOfCombinations++;
			if (CombinationsFinder::showResults) {
				for (int i = 0; i < index; i++)
					std::cout << arr[i] << " ";
				std::cout << std::endl;
			}
			return;
		}

		// Find the previous number stored in arr[] 
		// It helps in maintaining increasing order 
		int prev = (index == 0) ? 1 : arr[index - 1];

		// note loop starts from previous number 
		// i.e. at array location index - 1 
		for (int k = prev; k <= num; k++)
		{
			// next element of array is k 
			arr[index] = k;

			// call recursively with reduced number 
			findCombinationsUtil(arr, index + 1, num,
				reducedNum - k);
		}
	}

	/* Function to find out all combinations of
	positive numbers that add upto given number.
	It uses findCombinationsUtil() */
	void findCombinations(int n, bool showResults = true)
	{
		CombinationsFinder::showResults = showResults;

		// array to store the combinations 
		// It can contain max n elements 
		//int arr[n];

		//dynamic array
		int* arr = new int[n];

		//find all combinations 
		findCombinationsUtil(arr, 0, n, n);

		g_countOfCombinations--;
		delete[] arr;
	}
}


namespace CombinationsFinder_MultiThreaded {
	// Семафор для ограничения количества одновременно выполняемых потоков
	HANDLE sem;
	HANDLE* threads;
	bool showResults = true;

	int countOfCombinationsInMemory = 0;

	class Combination {
	public:
		std::vector<int> vector;
		int index;
		Combination(std::vector<int>& oldVector, int lastIndex) {
			SetVectorAndIndex(oldVector, lastIndex);
		}

		void SetVectorAndIndex(std::vector<int>& oldVector, int lastIndex) {
			vector = std::vector<int>();

			index = lastIndex;
			CopyVector(oldVector);
		}

		Combination() {
			vector = std::vector<int>();
			index = -1;
		}

		void AnalyseCombination() {
			
		}

		void CopyVector(std::vector<int>& source)
		{
			for (int i = 0; i < index; i++)
				vector.push_back(source[i]);
		}
	};
	
	int countOfResultsToSave = 0;

	int TakeResults()
	{
		EnterCriticalSection(&cs);
		int result = countOfResultsToSave;
		countOfResultsToSave = 0;
		LeaveCriticalSection(&cs);
		return result;
	}

	std::queue<Combination> combinations;
	bool endOfCalculations;
	const int MAX_INSTANCES_IN_MEMORY = 8000;
	/*    arr - array to store the combination
	index - next location in array
	num - given number
	reducedNum - reduced number */
	void findCombinationsUtil(int* arr, int index,
		int num, int reducedNum)
	{
		// Base condition 
		if (reducedNum < 0)
			return;

		// If combination is found, print it 
		if (reducedNum == 0)
		{
			//wait until all working threads clear the combinations buffer
			//if (combinations.size() > MAX_INSTANCES_IN_MEMORY) {
			//	while (combinations.size() > g_threadCount) {
			//		Sleep(5);
			//	}
			//}
			//if (showResults) {
			//	if (countOfCombinationsInMemory > MAX_INSTANCES_IN_MEMORY) {
			//		Sleep(5);
			//	}

			//	// add a combination to check if the condition is met
			//	Combination cmb(arr, index);
			//	combinations.push(cmb);
			//	countOfCombinationsInMemory++;
			//} else {
			countOfResultsToSave++;
			//}		  
			//remove the line
			//combinations.pop();
			return;
		}

		// Find the previous number stored in arr[] 
		// It helps in maintaining increasing order 
		int prev = (index == 0) ? 1 : arr[index - 1];

		// note loop starts from previous number 
		// i.e. at array location index - 1 
		for (int k = prev; k <= num; k++)
		{
			// next element of array is k 
			arr[index] = k;

			// call recursively with reduced number 
			findCombinationsUtil(arr, index + 1, num,
				reducedNum - k);
		}
	}

	DWORD WINAPI AnalyzeCombination(LPVOID lpParam)
	{
		// lpParam not used in this example
		UNREFERENCED_PARAMETER(lpParam);

		DWORD dwWaitResult;
		BOOL bContinue = TRUE;

		while (!endOfCalculations)
		{
			// Try to enter the semaphore gate.

			dwWaitResult = WaitForSingleObject(
				sem,				// handle to semaphore
				INFINITE);			// infinite time-out interval

			switch (dwWaitResult)
			{
				// The semaphore object was signaled.
				case WAIT_OBJECT_0:
					
					// Perform task
					// task start {
					if (countOfResultsToSave > 0) {
						g_countOfCombinations += TakeResults();
						Sleep(100);
					}
					
					if (showResults) {
						
						//if (countOfCombinationsInMemory > 0) {
						if (!combinations.empty()) {
							Combination cmb = Combination(combinations.front().vector, combinations.front().index);

							//int sum = 0;
							//// If combination is found, print it 
							//for (int i = 0; i < cmb->index; i++)
							//{
							//	sum += cmb->vector[i];
							//}

							//if (sum == g_targetNumber) 
							{
								//EnterCriticalSection(&cs);
								//std::cout << g_countOfCombinations << ") ";

								//for (int i = 0; i < cmb->index; i++)
								//	std::cout << (*cmb->vector)[i] << " ";
								int i = 0;

								std::string output;
								for (auto it = cmb.vector.begin(); it != cmb.vector.end(); it++)
								{
									output.append(std::to_string(*it));
									output.append(" ");
								}

								EnterCriticalSection(&cs);
								std::cout << output << std::endl;
								LeaveCriticalSection(&cs);
								//LeaveCriticalSection(&cs);

								//break;
							}
							combinations.pop();	countOfCombinationsInMemory--;
						}
					}

					// Release the semaphore when task is finished

					if (!ReleaseSemaphore(
						sem,			// handle to semaphore
						1,				// increase count by one
						NULL))			// not interested in previous count
					{
						printf("ReleaseSemaphore error: %d\n", GetLastError());
					}
					break;

				// task end }
				// The semaphore was nonsignaled, so a time-out occurred.
				case WAIT_TIMEOUT:
					printf("Thread %d: wait timed out\n", GetCurrentThreadId());
					break;
			}
		}
		return TRUE;
	}

	/* Function to find out all combinations of
	positive numbers that add upto given number.
	It uses findCombinationsUtil() 
	n – target number
	showResults – output reulting string into std::cout
	*/
	void findCombinations(int n, bool showResults = true)
	{
		countOfCombinationsInMemory = 0;
		g_countOfCombinations = 0;
		g_targetNumber = n;
		CombinationsFinder_MultiThreaded::showResults = showResults;
		sem = CreateSemaphore(
			NULL,           // default security attributes
			1,				// initial count
			g_threadCount,  // maximum count
			NULL);          // unnamed semaphore

		if (sem == NULL)
		{
			printf("CreateSemaphore error: %d\n", GetLastError());
		}

		// Initialize the critical section one time only.
		if (!InitializeCriticalSectionAndSpinCount(&cs,
			0x00000400))
			return;


		// Create worker threads
		DWORD ThreadID;
		threads = new HANDLE[g_threadCount];
		for (int i = 0; i < g_threadCount; i++)
		{
			threads[i] = CreateThread(
				NULL,       // default security attributes
				0,          // default stack size
				(LPTHREAD_START_ROUTINE)AnalyzeCombination,
				NULL,       // no thread function arguments
				0,          // default creation flags
				&ThreadID); // receive thread identifier

			if (threads[i] == NULL)
			{
				printf("CreateThread error: %d\n", GetLastError());
				return;
			}
		}
		endOfCalculations = false;

		//dynamic container (vector)
		int* arr = new int[n];

		//g_simpleTimer = SimpleTimer::SimpleTimer();

		//find all combinations 
		findCombinationsUtil(arr, 0, n, n);

		//g_millisecondsSpentOutputStr = g_simpleTimer.Stop();

		endOfCalculations = true;

		// Wait for all threads to terminate
		WaitForMultipleObjects(g_threadCount, threads, TRUE, INFINITE);

		// Close thread and semaphore handles
		for (int i = 0; i < g_threadCount; i++)
			CloseHandle(threads[i]);

		delete[] threads;
		CloseHandle(sem);

		// Release resources used by the critical section object.
		DeleteCriticalSection(&cs);

		if (countOfResultsToSave > 0) {
			g_countOfCombinations += TakeResults();
			Sleep(100);
		}
		g_countOfCombinations--;
		delete arr;
	}
}

namespace SemaphoreMSDN {

#define MAX_SEM_COUNT 10
#define THREADCOUNT 12

	HANDLE ghSemaphore;

	DWORD WINAPI ThreadProc(LPVOID);

	int main(void)
	{
		HANDLE aThread[THREADCOUNT];
		DWORD ThreadID;
		int i;

		// Create a semaphore with initial and max counts of MAX_SEM_COUNT

		ghSemaphore = CreateSemaphore(
			NULL,           // default security attributes
			MAX_SEM_COUNT,  // initial count
			MAX_SEM_COUNT,  // maximum count
			NULL);          // unnamed semaphore

		if (ghSemaphore == NULL)
		{
			printf("CreateSemaphore error: %d\n", GetLastError());
			return 1;
		}

		// Create worker threads

		for (i = 0; i < THREADCOUNT; i++)
		{
			aThread[i] = CreateThread(
				NULL,       // default security attributes
				0,          // default stack size
				(LPTHREAD_START_ROUTINE)ThreadProc,
				NULL,       // no thread function arguments
				0,          // default creation flags
				&ThreadID); // receive thread identifier

			if (aThread[i] == NULL)
			{
				printf("CreateThread error: %d\n", GetLastError());
				return 1;
			}
		}

		// Wait for all threads to terminate

		WaitForMultipleObjects(THREADCOUNT, aThread, TRUE, INFINITE);

		// Close thread and semaphore handles

		for (i = 0; i < THREADCOUNT; i++)
			CloseHandle(aThread[i]);

		CloseHandle(ghSemaphore);

		return 0;
	}

	DWORD WINAPI ThreadProc(LPVOID lpParam)
	{

		// lpParam not used in this example
		UNREFERENCED_PARAMETER(lpParam);

		DWORD dwWaitResult;
		BOOL bContinue = TRUE;

		while (bContinue)
		{
			// Try to enter the semaphore gate.

			dwWaitResult = WaitForSingleObject(
				ghSemaphore,   // handle to semaphore
				0L);           // zero-second time-out interval

			switch (dwWaitResult)
			{
				// The semaphore object was signaled.
			case WAIT_OBJECT_0:
				// TODO: Perform task
				printf("Thread %d: wait succeeded\n", GetCurrentThreadId());
				bContinue = FALSE;

				// Simulate thread spending time on task
				Sleep(5);

				// Release the semaphore when task is finished

				if (!ReleaseSemaphore(
					ghSemaphore,  // handle to semaphore
					1,            // increase count by one
					NULL))       // not interested in previous count
				{
					printf("ReleaseSemaphore error: %d\n", GetLastError());
				}
				break;

				// The semaphore was nonsignaled, so a time-out occurred.
			case WAIT_TIMEOUT:
				printf("Thread %d: wait timed out\n", GetCurrentThreadId());
				break;
			}
		}
		return TRUE;
	}
}



//Дано натуральное число N.Его можно представить как сумму других
//натуральных чисел, меньших N, например 3 можно представить как :
//3 = 1 + 1 + 1
//3 = 2 + 1
//Найти количество уникальных разложений.Разложения, отличающиеся
//порядком слагаемых, считаются одинаковыми(например 3 = 2 + 1 и 3 = 1 + 2).
//Реализовать программу в файле expr.cpp
//Входные данные : файл input.txt.
//Первая строка — натуральное число T, количество потоков рабочего пула для
//решения задачи.
//Вторая строка — натуральное число N(2 <= N <= 100)

//Выходные данные : файл output.txt, содержит 3 строки
//Первая строка — натуральное число, количество потоков(как во входном
//	файле).
//	Вторая строка — натуральное число N(как во входном файле).
//	Третья строка — решение задачи, количество уникальных разложений числа
//	на слагаемые.
//	Выходные данные : файл time.txt.Содержит единственное число — время
//	поиска ответа, в миллисекундах.Замеряться должно только время работы
//	алгоритма, без учета времени загрузки входных данных и создания потоков.

bool InitializeStreams()
{
	try {
		g_inputStream = std::ifstream();
		g_inputStream.open("input.txt");

		g_outputStream = std::ofstream();
		g_outputStream.open("output.txt", std::ios::trunc);

		return true;
	}
	catch (...) {
		return false;
	}
}

void Output()
{
	g_outputStream << g_threadCount;
	g_outputStream << std::endl;

	g_outputStream << g_targetNumber;
	g_outputStream << std::endl;

	g_outputStream << g_millisecondsSpentOutputStr;
	g_outputStream << std::endl;
}

void CloseStreams() 
{
	g_inputStream.close();
	g_outputStream.close();
}

bool InitializeInputData() {
	//std::string firstString;

	//g_threadCount = 2;

	//std::string secondString;
	//g_minNumber = 2;
	//g_maxNumber = 100;

	g_threadCount = -1;
	g_targetNumber = -1;

	std::string tempLine;
	std::getline(g_inputStream, tempLine);
	g_threadCount = std::stoi(tempLine);

	std::getline(g_inputStream, tempLine);
	g_targetNumber = std::stoi(tempLine);;

	if (g_threadCount == -1 || g_targetNumber == -1)
	{
		return false;
	}
	return true;
}

int main()
{
	g_countOfCombinations = 0;

	if (
		//false
		!InitializeStreams()
		) {
		std::cout << "File \"input.txt\" doesn't exist." << std::endl;
		return 1;
	}

	if (!InitializeInputData()) {
		std::cout << "Input file is currupted... quiting..." << std::endl;
		return 1;
	}

	CombinationsFinder::findCombinations(g_targetNumber, true);
	std::cout << "Count of combinations " << g_countOfCombinations << std::endl;

	g_countOfCombinations = 0;
	CombinationsFinder_MultiThreaded::findCombinations(g_targetNumber, false);
	std::cout << "Count of combinations " << g_countOfCombinations << std::endl;

	//std::string timeSpent = simpleTimer.Stop();
	//g_outputStream.write(timeSpent.c_str(), timeSpent.length());

	Output();
	CloseStreams();
	
	system("pause");
    return 0;
}

