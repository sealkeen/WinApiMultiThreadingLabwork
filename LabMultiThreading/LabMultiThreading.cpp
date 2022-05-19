// LabMultiThreading.cpp: ���������� ����� ����� ��� ����������� ����������.
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

std::vector<int> g_possibleCurrentNumbers;

int g_countOfCombinations = 0;
int g_targetNumber = 0;

namespace CombinationsFinder {

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

			for (int i = 0; i < index; i++)
				std::cout << arr[i] << " ";
			std::cout << std::endl;
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
	void findCombinations(int n)
	{
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


namespace CombinationsFinder_MultiThreaded2 {
	// Семафор для ограничения количества одновременно выполняемых потоков
	HANDLE sem;
	HANDLE* threads;


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

	std::queue<Combination> combinations;
	bool endOfCalculations;
	/*    arr - array to store the combination
	index - next location in array
	num - given number
	reducedNum - reduced number */
	void findCombinationsUtil(std::vector<int> arr, int index,
		int num, int reducedNum)
	{
		// Base condition 
		if (reducedNum < 0)
			return;

		// If combination is found, print it 
		if (reducedNum == 0)
		{
			//wait until all working threads clear the combinations buffer
			while (combinations.size() > g_threadCount) {
				Sleep(5);
			}
			// add a combination to check if the condition is met
			Combination cmb(arr, index);
			combinations.push(cmb);
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

	DWORD WINAPI AnalyseCombination(LPVOID lpParam)
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
					if (!combinations.empty()) {
						Combination cmb = Combination(combinations.front().vector, combinations.front().index);
						//EnterCriticalSection(&cs);

						//int sum = 0;
						//// If combination is found, print it 
						//for (int i = 0; i < cmb->index; i++)
						//{
						//	sum += cmb->vector[i];
						//}

						//if (sum == g_targetNumber) 
						{
							g_countOfCombinations++;

							//EnterCriticalSection(&cs);
							std::cout << g_countOfCombinations << ") ";

							int i = 0;
							//for (int i = 0; i < cmb->index; i++)
							//	std::cout << (*cmb->vector)[i] << " ";
							for (auto it = cmb.vector.begin(); it != cmb.vector.end(); it++)
							{
								std::cout << *it << " ";
							}

							std::cout << std::endl;
							//LeaveCriticalSection(&cs);

							//break;
						}
						combinations.pop();
						//LeaveCriticalSection(&cs);
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
	It uses findCombinationsUtil() */
	void findCombinations(int n)
	{
		g_countOfCombinations = 0;
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
				(LPTHREAD_START_ROUTINE)AnalyseCombination,
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
		// array to store the combinations 
		// It can contain max n elements 
		//int arr[n];

		//dynamic array
		std::vector<int> arr(n);

		//find all combinations 
		findCombinationsUtil(arr, 0, n, n);

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
		//g_countOfCombinations--;
	}
}

namespace SemaphoreTry {

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

namespace CombinationsFinder_MultiThreaded {

	typedef struct Arguments {
		std::vector<int> arg1; int arg2; int arg3; int arg4;
	};

	//std::vector<std::vector<int>>* g_tempArraysWithCombinations = 
	//new std::vector<std::vector<int>>( );
	//delete g_tempArraysWithCombinations;
	std::vector<std::vector<int>> g_combinationsArrays;
	std::vector<Arguments> g_arraysWithArguments;
	int g_threadIndex = 0;

	//void 

	void ShowArrays()
	{
		int i = 0;
		for (auto arrayIt = g_combinationsArrays.begin(); arrayIt != g_combinationsArrays.end(); arrayIt++, i++) {
			for (auto combinationIt = arrayIt->begin(); combinationIt != arrayIt->end(); combinationIt++)
			{
				std::cout << *combinationIt << " ";
			}
			std::cout << std::endl;
		}
	}

	//arr - array to store the combination
	std::vector<int> arr;
	std::vector<int> indexes;
	std::vector<int> reducedNumbers;
	std::vector<int> previousNumbers;
	int targetNumber;


	bool ReturnCondition() {
		// Base condition 
		if (reducedNumbers.back() < 0) {
			return true;
		}

		// If combination is found, print it 
		if (reducedNumbers.back() == 0) {
			g_countOfCombinations++;

			for (int i = 0; i < indexes.back(); i++)
				std::cout << arr[i] << " ";
			std::cout << std::endl;
			return true;
		}
		return false;
	}

	//index - next location in array
	//num - given number
	//reducedNum - reduced number */
	void findCombinationsUtil()
	{
			// Find the previous number stored in arr[] 
			// It helps in maintaining increasing order 
			previousNumbers.push_back((indexes.back() == 0) ? 1 : arr[indexes.back() - 1]);

			// note loop starts from previous number 
			// i.e. at array location index - 1 
			for (int k = previousNumbers.back(); k <= targetNumber; k++)
			{
				previousNumbers.pop_back();
				// next element of array is k 
				arr[indexes.back()] = k;

				while (true) {
					if (!ReturnCondition()) {
						previousNumbers.push_back((indexes.back() == 0) ? 1 : arr[indexes.back() - 1]);
						indexes.push_back(indexes.back() + 1);
						// call recursively with reduced number 
						reducedNumbers.push_back(reducedNumbers.back() - k);

						for (int j = previousNumbers.back(); j <= targetNumber; j++)
						{
							previousNumbers.pop_back();
							arr[indexes.back()] = j;
						}
					} else {
						reducedNumbers.pop_back();
						indexes.pop_back();
						previousNumbers.pop_back();
						break;
					}
				} 

				// save arguments that for the functions to call
			}
	}

	/* Function to find out all combinations of
	positive numbers that add upto given number.
	It uses findCombinationsUtil() */
	void findCombinations(int n)
	{
		// array to store the combinations 
		// It can contain max n elements 
		//int arr[n];

		//dynamic array
		//int* arr = new int[n];

		//vector
		arr = std::vector<int>();
		arr.resize(n);

		//resized count of arrays in accordance with count of threads
		g_combinationsArrays.resize(g_threadCount);
		g_arraysWithArguments.resize(g_threadCount);

												//��������, 5
		for (int threadIndex = 0; threadIndex < g_threadCount; threadIndex++)
		{
			g_combinationsArrays[threadIndex].resize(n);
		}

		//debug
		//ShowArrays();
		//return;

		//find all combinations 
		targetNumber = n;
		reducedNumbers.push_back(n);
		indexes.push_back(0);
		findCombinationsUtil();

		g_countOfCombinations--;
	}
}

namespace SimpleTimer {
	class SimpleTimer
	{
	public:

		SimpleTimer::SimpleTimer()
		{
			start = std::chrono::high_resolution_clock::now();
		}

		std::string SimpleTimer::Stop()
		{
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			float result = duration.count();
			std::string strResult;

			strResult.append("Time spent: ");
			strResult.append(std::to_string(result));
			strResult.append("seconds");
			strResult.append("\n");
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

bool InitializeStreams()
{
	g_inputStream = std::ifstream();
	g_inputStream.open("input.txt");

	g_outputStream = std::ofstream();
	g_outputStream.open("output.txt");

	return true;
}


void CloseStreams() 
{
	g_inputStream.close();
	g_outputStream.close();
}


void InitializeInputData() {
	std::string firstString;

	//������� �������������� �� ������ � �����
	//...
	//���������� ������� �������� ����
	g_threadCount = 2;

	std::string secondString;
	//������� �������������� �� ����� � �����
	//...
	g_minNumber = 2;
	g_maxNumber = 100;
}

bool CheckForSum()
{
	//TODO:
	return true;
}

//��������, ��� ����� ���� ��� �������
bool IdenticalPairExistsAlready()
{
	//TODO:
	return true;
}

void HandleNumbers()
{
	//enter semaphor
	//waitforsingleobject()

	g_countOfCombinations++;

	//leave semaphor
	//release_semaphor
}

void AddElementNumbers(std::vector<int> pairToAdd)
{
	EnterCriticalSection(&cs);

	//g_allPairs.push_back(g_possibleCurrentNumbers);

	LeaveCriticalSection(&cs);
}

void FindCombination(int* combination, int maxIndex)
{

}


int main()
{
	SimpleTimer::SimpleTimer simpleTimer;
	g_countOfCombinations = 0;

	if (false
		//!InitializeStreams()
		) {
		std::cout << "����� \"input.txt\" �� �������." << std::endl;
		//return 0;
	}

	InitializeInputData();

	//CombinationsFinder_MultiThreaded::findCombinations(5);
	//CombinationsFinder::findCombinations(15);

	CombinationsFinder_MultiThreaded2::findCombinations(10);

	//Calculate(5);

	std::cout << "Count of combinations " << g_countOfCombinations << std::endl;

	//std::string timeSpent = simpleTimer.Stop();
	//g_outputStream.write(timeSpent.c_str(), timeSpent.length());
	simpleTimer.~SimpleTimer();

	//CloseStreams();
	system("pause");
    return 0;
}

