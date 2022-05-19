#pragma once

#include <windows.h>
#include <vector>
#include <queue>
#include <string>
#include <iostream>
#include <list>

#define external
#ifdef external
int g_threadCount = 5;
int g_countOfCombinations = 0;
int g_targetNumber = 10;
CRITICAL_SECTION cs;
#endif //external

namespace NewNewSolution {
	void FindAll() {
		std::list<int> list;
		for (int i = 0; i < g_targetNumber; i++) {
			list.push_back(1);
		}

		int difference = 0;
		for (int i = 0; i < g_targetNumber; i++) {
			//difference = NewSolution::Difference(list);
			if (difference > 0)
				break;
			else if (difference < 0)
				break;
			else if (difference == 0)
				break;
		}
	}
}

namespace CombinationsFinder_MT2_Try {
	bool showResults = true;
	bool shutDownThreads = false;
	bool endOfCalculations = false;
	HANDLE sem;

	std::vector<int> countForEachIndex;

	void InitializeIndexes() {
		countForEachIndex = std::vector<int>();
		for ( int i = 0; i <= g_targetNumber; i++ )
		{
			countForEachIndex.push_back(0);
		}
	}

	/*    arr - array to store the combination
	index - next location in array
	num - given number
	reducedNum - reduced number */
	void findCombinationsUtil(int** arr, int index,
		int num, int reducedNum, int permanentIndex, int arrayIndex)
	{
		//if()
		// Base condition 
		if (reducedNum < 0)
			return;

		// If combination is found, print it 
		if (reducedNum == 0)
		{
			g_countOfCombinations++;
			if (index == permanentIndex) {
				countForEachIndex[permanentIndex] = countForEachIndex[permanentIndex] + 1;
			}
			return;
		}

		// Find the previous number stored in arr[] 
		// It helps in maintaining increasing order 
		int prev = (index == 0) ? 1 : arr[arrayIndex][index - 1];

		// note loop starts from previous number 
		// i.e. at array location index - 1 
		for (int k = prev; k <= num; k++)
		{
			// next element of array is k 
			arr[arrayIndex][index] = k;

			// call recursively with reduced number 
			findCombinationsUtil(arr, index + 1, num,
				reducedNum - k, permanentIndex, arrayIndex);
		}
	}

	/* Function to find out all combinations of
	positive numbers that add upto given number.
	It uses findCombinationsUtil() */
	void findCombinations(int n, bool showResults = true)
	{
		n = g_targetNumber;
		//CombinationsFinder::showResults = showResults;
		g_countOfCombinations = 0;
		InitializeIndexes();

		// array to store the combinations 
		// It can contain max n elements 
		//int arr[n];

		//dynamic arrays
		int** dynamicArrays = new int*[g_targetNumber];
		for (int i = 0; i <= g_targetNumber; i++)
		{
			int* newArray = new int[g_targetNumber];
			dynamicArrays[i] = newArray;
		}

		int arrayIndex = 0;
		for (int i = (g_targetNumber - 1); i >= 0; i++) {
			findCombinationsUtil(dynamicArrays, 0, n, n, i, arrayIndex++);
		}

		for (int i = 0; i < g_targetNumber; i++)
		{
			g_countOfCombinations = +countForEachIndex.back();
			countForEachIndex.pop_back();
		}

		//free up the memory
		for (int i = 0; i <= g_targetNumber; i++)
		{
			delete[] dynamicArrays[i];
		}
		delete[] dynamicArrays;

		g_countOfCombinations--;
	}


	DWORD WINAPI AnalyseCombination(LPVOID lpParam)
	{
		// lpParam not used in this example
		UNREFERENCED_PARAMETER(lpParam);

		DWORD dwWaitResult;
		BOOL bContinue = TRUE;

		while (!endOfCalculations && !shutDownThreads)
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


				// The semaphore was nonsignaled, so a time-out occurred.
			case WAIT_TIMEOUT:
				printf("Thread %d: wait timed out\n", GetCurrentThreadId());
				break;
			}
		}
		return TRUE;
	}

}

//void FindSolution()
//{
//	int maxValue = g_targetNumber;
//	int maxIndex = g_targetNumber - 1;
//	for (; maxValue; maxValue--) {
//
//	}
//
//}
//
//void Decrease(int* values; int index)
//{
//	for (int i = index; i >= 0; i--)
//	{
//
//	}
//}

namespace CombinationsFinder_MultiThreaded {
	// —емафор дл€ ограничени€ количества одновременно выполн€емых потоков
	HANDLE sem;
	HANDLE* threads;
	bool showResults = true;
	bool shutDownThreads = false;

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

	std::queue<Combination> combinations;
	bool endOfCalculations;
	const int MAX_INSTANCES_IN_MEMORY = 8000;
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
			//if (combinations.size() > MAX_INSTANCES_IN_MEMORY) {
			//	while (combinations.size() > g_threadCount) {
			//		Sleep(5);
			//	}
			//}

			if (countOfCombinationsInMemory > MAX_INSTANCES_IN_MEMORY) {
				Sleep(5);
			}

			// add a combination to check if the condition is met
			Combination cmb(arr, index);
			combinations.push(cmb);
			countOfCombinationsInMemory++;

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

	DWORD WINAPI AnalyseCombination(LPVOID lpParam)
	{
		// lpParam not used in this example
		UNREFERENCED_PARAMETER(lpParam);

		DWORD dwWaitResult;
		BOOL bContinue = TRUE;

		while (!endOfCalculations && !shutDownThreads)
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
				if (countOfCombinationsInMemory > 0) {
					//if (!combinations.empty()) {
					Combination cmb = Combination(combinations.front().vector, combinations.front().index);

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
						//std::cout << g_countOfCombinations << ") ";

						//for (int i = 0; i < cmb->index; i++)
						//	std::cout << (*cmb->vector)[i] << " ";
						if (CombinationsFinder_MultiThreaded::showResults) {
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
						}
						//LeaveCriticalSection(&cs);

						//break;
					}
					combinations.pop();	countOfCombinationsInMemory--;
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
	It uses findCombinationsUtil()
	n Ц target number
	showResults Ц output reulting string into std::cout
	*/
	void findCombinations(int n, bool showResults = true)
	{
		shutDownThreads = false;
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

		//dynamic container (vector)
		std::vector<int> arr(n);

		//find all combinations 
		findCombinationsUtil(arr, 0, n, n);

		endOfCalculations = true;
		shutDownThreads = true;
		// Wait for all threads to terminate
		WaitForMultipleObjects(g_threadCount, threads, TRUE, INFINITE);

		// Close thread and semaphore handles
		for (int i = 0; i < g_threadCount; i++)
			CloseHandle(threads[i]);

		delete[] threads;
		CloseHandle(sem);

		// Release resources used by the critical section object.
		DeleteCriticalSection(&cs);

		if (countOfCombinationsInMemory > 0) {
			g_countOfCombinations += countOfCombinationsInMemory;
			Sleep(100);
		}
		g_countOfCombinations--;
	}
}

// The base code for our application semaphore
// From MSSDN
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

void Run_MT2(bool showResults = false) {
	g_countOfCombinations = 0;
	//SimpleTimer::SimpleTimer simpleTimer;
	//CombinationsFinder_MT2_Try::findCombinations(g_targetNumber, showResults);
	std::cout << "Count of combinations " << g_countOfCombinations << std::endl;
}

int Fact(int number)
{
	int result = 1;
	for (int i = 1;i <= number;i++) {
		result = result * i;
	}
	return result;
}

unsigned long Soch()
{
	int result = 0;
	unsigned int n = g_targetNumber;
	int k = 0;
	for (int i = 1; i <= g_targetNumber; i++)
		//result += pow(k, k);
		k += i;

	return (Fact((n + n - 1) / Fact(n)*Fact(n - 1)));

	return pow(n, Fact(n));

	return result;
}