#include "stdafx.h"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <chrono>
#include <queue>
#include <list>

#define SHOW_RESULTS

CRITICAL_SECTION cs;

//using namespace std;
std::ifstream g_inputStream;
std::ofstream g_outputStream;

int g_threadCount;
int g_minNumber = 2, g_maxNumber = 100;


std::vector<int> g_possibleCurrentNumbers;

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
			stopped = false;
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

		std::string SimpleTimer::StopMilliseconds()
		{
			stopped = true;
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
			std::string strResult;

			strResult.append(std::to_string(millis));
			return strResult;
		}

		SimpleTimer::~SimpleTimer()
		{
			if (stopped)
				return;
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			float result = duration.count();
			std::cout << "Time spent: " << result << " seconds" << std::endl;
		}
	private:
		std::chrono::time_point<std::chrono::steady_clock> start, end;
		std::chrono::duration<float> duration;
		bool stopped;
	};
}

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

namespace CombinationsFinder_MTT_Try {
	bool showResults = true;
	bool shutDownThreads = false;
	bool endOfCalculations = false;
	HANDLE sem;

	std::vector<std::vector<int>> arraysToAnalyze;

	/*    arr - array to store the combination
	index - next location in array
	num - given number
	reducedNum - reduced number */
	void findCombinationsUtil(std::vector<int> arr, int index,
		int num, int reducedNum)
	{
		//if()
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
			findCombinationsUtil(std::vector<int>(arr), index + 1, num,
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
		std::vector<int> arr = std::vector<int>(n);

		//find all combinations 
		findCombinationsUtil(arr, 0, n, n);

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


namespace CombinationsFinder_MultiThreaded {
	// Семафор для ограничения количества одновременно выполняемых потоков
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
					if(countOfCombinationsInMemory > 0) {
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
	n – target number
	showResults – output reulting string into std::cout
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
	try {
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
	}
	catch (...) {
		return false;
	}
	
	return true;
}

void SimulateInitialization() {
	g_threadCount = 2;
	g_targetNumber = 10;
}


void RunMultiThreaded(bool showResults = false) {
	SimpleTimer::SimpleTimer simpleTimer = SimpleTimer::SimpleTimer();
	//simpleTimer = SimpleTimer::SimpleTimer();
	CombinationsFinder::findCombinations(g_targetNumber, showResults);
	std::cout << "Count of combinations " << g_countOfCombinations << std::endl;
	g_millisecondsSpentOutputStr = simpleTimer.StopMilliseconds();
	//std::string timeSpent = simpleTimer.Stop();
	//g_outputStream.write(timeSpent.c_str(), timeSpent.length());
}

namespace NewSolution {
	typedef std::vector<int> SolutionIntContainer;
	typedef int ReverseIterator;

	std::list<SolutionIntContainer> combinations;

	void ShowList(SolutionIntContainer& list, std::string* additionalContents = nullptr)
	{
	#ifdef SHOW_RESULTS
		for (auto i = list.begin(); i != list.end(); i++) {
			std::cout << (*i) << " ";
		}
		if (additionalContents != nullptr) {
			std::cout << " \t " << *additionalContents << " \t ";
		}
		std::cout << std::endl;
	#endif
	}

	void ShowCombination( SolutionIntContainer& container, std::string* additionalContents = nullptr) {
	#ifdef SHOW_RESULTS
		std::cout << g_countOfCombinations << ") \t";
		ShowList(container, additionalContents);
	#endif
	}

	bool SufficientValueDifference(int left, int right, int difference) {
		if (right > (left + 1))
			return true;
		return false;
	}

	bool RebuildCombination(SolutionIntContainer& targetList/*, ReverseIterator rightIndex,*/) {
		// strictly more than 0 because we move towards left border
		// and check for the previous (right - 1) element

		int rightIndex = targetList.size() - 1;
		int leftIndex = 0;

		if (g_countOfCombinations == 44)
			std::string();

		if (!SufficientValueDifference(targetList[leftIndex], targetList[rightIndex], 1))
			return false;

		while (leftIndex < (rightIndex-1)) {
			if (!SufficientValueDifference(targetList[leftIndex], targetList[rightIndex-1], 1))
				break;
			else
				rightIndex--;
		}

		while ((leftIndex+1) < rightIndex) {
			if (!SufficientValueDifference(targetList[leftIndex+1], targetList[rightIndex], 1))
				break;
			else
				leftIndex++;
		}

		targetList[rightIndex] = targetList[rightIndex] - 1;
		targetList[leftIndex] = targetList[leftIndex] + 1;

		std::string indexesStr = std::string("rightIndex = ") + std::to_string(rightIndex) +
			+ " ; " +
			std::string("leftIndex = ") + std::to_string(leftIndex) +
			std::string(";");

		ShowCombination(targetList, &indexesStr);
		g_countOfCombinations++;

		return true;
	}

	bool RebuildOnce(SolutionIntContainer& targetList, ReverseIterator rightIndex, int increaseIncludingBorder) {
		// strictly more than 0 because we move towards left border
		// and check for the previous (right - 1) element
		for (; rightIndex > 0; rightIndex--) {
			for (int leftIndex = rightIndex - 1; leftIndex >= 0; leftIndex--) {
				
				// if two near elements are equal 
				if ( targetList[rightIndex] == targetList[leftIndex] )
					continue;

				// if the right element is greater than the left element by 2
				if ( targetList[rightIndex] > (targetList[leftIndex] + 1) )  {
					// if increasing the left element doesn't make it more than the right element
					if ( (targetList[(leftIndex)] + 1) <= targetList[rightIndex] ) {
						//int prevIndex = (rightIndex - 1);
						//if ( prevIndex < targetList.size() && prevIndex >= 0 ) {
							if (targetList[leftIndex] + 1 <= increaseIncludingBorder) {
								targetList[rightIndex] = targetList[rightIndex] - 1;
								targetList[leftIndex] = targetList[leftIndex] + 1;
						//	}
						}
						//RebuildNext(targetList, left);
						return true;
					}
				}
			}
		}
		return false;
	}

	//bool RebuildNext(std::list<int>& targetList, ListIterator right) {
	//	for (auto left = std::next(right); left != targetList.rend(); left++) {
	//		if ((*right) == (*left))
	//			continue;
	//		// if the left element is greater than the right element by 2
	//		if ((*right) > (*left) + 1) {
	//			(*right) = (*right) - 1;
	//			(*left) = (*left) + 1;
	//			RebuildNext(targetList, left);
	//			return true;
	//		}
	//	}
	//}

	int Difference(int sum)
	{
		return g_targetNumber - sum;
	}

	int Difference(SolutionIntContainer& targetList) {
		int sum = 0;
		for (auto it = targetList.rbegin(); it != targetList.rend(); it++) {
			sum += (*it);
		}
		return Difference(sum);
	}

	void InitializeCombinations()
	{
		for (int i = g_targetNumber; i > 1; i--)
		{
			SolutionIntContainer newList = SolutionIntContainer();
			for (int k = 0; k < i; k++) {
				newList.push_back(1);
			}
			combinations.push_back(newList);
		}
	}

	// if ReverseIterator is list<int>::reverse_iterator
	//ReverseIterator CopyIterator(SolutionIntContainer& source, ReverseIterator& sourceIterator, SolutionIntContainer& target) {
	//	ReverseIterator targetIterator = target.rbegin();
	//	std::advance(targetIterator, std::distance(source.rbegin(), sourceIterator));
	//	return targetIterator;
	//}

	int CopyIterator(int& sourceIterator) {
		int targetIterator = sourceIterator;
		return targetIterator;
	}

	//Debug
	void ShowLists()
	{
		for (auto it1 = combinations.begin(); it1 != combinations.end(); it1++)
		{
			for(auto it2 = it1->begin(); it2 != it1->end() ; it2++){
				std::cout << (*it2) << " ";
			}
			std::cout << std::endl;
		}
	}

	void FindAllCombinationsForAList(SolutionIntContainer& targetList) {
		int size = targetList.size();
		auto back = targetList.rbegin();
		*back = size;
		for (auto it = targetList.rbegin(); it != targetList.rend(); it++) {
			//(*it) = size--;
			std::cout << (*it) << " ";

		}
		std::cout << std::endl;
	}

	void FindAllFittingSums(SolutionIntContainer targetList, ReverseIterator iterator, /*int reducedNumber,*/ bool firstRun = false) {
		int rebuiltHere = 0;

		//auto right = firstRun ? targetList.rbegin() : next(iterator);
		int rightIndex = firstRun ? targetList.size() - 1 : iterator - 1;
		//TODO: remove assignment
		firstRun = false;
		//for (; rightIndex > 0; rightIndex--) {
			int left = (rightIndex-1);
			int difference = 0;
			while ((difference = Difference(targetList)) > 0)
			{
				//if (rightIndex == iterator) {
				targetList[rightIndex]++;
				//}
				//else if ((targetList[rightIndex] + 1) <= targetList[iterator]) {
				//	targetList[rightIndex]++;
				//}
			}

			if (rightIndex == iterator) {
				g_countOfCombinations++;
				ShowCombination(targetList);
			}
			//rebuiltHere++;

			//if (rebuiltHere > 0) {
			// first solution found
			//}

			bool razmazano = false;
			while (
				razmazano =
				//NewSolution::RebuildOnce(targetList, rightIndex, reducedNumber)
				NewSolution::RebuildCombination(targetList)
				) {
				// new rebuilt solution found
				//ShowCombination(targetList);
				rebuiltHere++;
				//g_countOfCombinations++;
				//ReverseIterator newListIterator = CopyIterator(rightIndex);
				//RebuildCombination(targetList);
				//FindAllFittingSums(SolutionIntContainer(targetList), newListIterator, /*reducedNumber - targetList[rightIndex],*/ false);
			}

			//if (razmazano)
			//	return;
			//if (rebuiltHere > 0 && !razmazano) {
			//	g_countOfCombinations++;
			//	ShowCombination(targetList);
			//}
			//offset++;
		//}
	}
}

void Test()
{
	NewSolution::SolutionIntContainer lst; lst.push_back(1); lst.push_back(1); lst.push_back(1); lst.push_back(1); lst.push_back(4);
	NewSolution::ShowList(lst);
	NewSolution::RebuildOnce(lst, lst.size() - 1, g_targetNumber);
	NewSolution::ShowList(lst);
	NewSolution::RebuildOnce(lst, lst.size() - 1, g_targetNumber);
	NewSolution::ShowList(lst);
	NewSolution::RebuildOnce(lst, lst.size() - 1, g_targetNumber);
	NewSolution::ShowList(lst);
}

void TestFindAllFittings()
{
	g_targetNumber = 8;
	NewSolution::SolutionIntContainer lst; lst.push_back(1); lst.push_back(1); lst.push_back(1); lst.push_back(1); lst.push_back(1);
		NewSolution::ShowList(lst);
	NewSolution::FindAllFittingSums(lst, lst.size() - 1, /*g_targetNumber, */true);
}

void RunNewSolution(bool showResults = false) {
	//g_targetNumber = targetNumber;
	g_countOfCombinations = 0;
	NewSolution::InitializeCombinations();
	
	//int countOfCombinations = 0;
	//NewSolution::ShowLists();
	for (auto i = NewSolution::combinations.begin(); i != NewSolution::combinations.end(); i++)
	{
		if (NewSolution::Difference(*i) == 0) {
			g_countOfCombinations++;
			NewSolution::ShowCombination((*i));
			continue;
		}
		NewSolution::FindAllFittingSums(*i, i->size()-1, /*g_targetNumber, */true);
		//sort((*i)->begin(), i->end());
		//i->erase(unique(i->begin(), i->end()), i->end());
		//countOfCombinations += i->size();
	}

	std::cout << "Count of combinations " << g_countOfCombinations /*+ g_targetNumber-1*/ << std::endl;
	std::cout << "*** New Solution Execution Completed ***" << std::endl;
}

void Run(bool showResults = false) {
	g_countOfCombinations = 0;
	SimpleTimer::SimpleTimer simpleTimer;
	CombinationsFinder::findCombinations(g_targetNumber, showResults);
	std::cout << "Count of combinations " << g_countOfCombinations << std::endl;
}

void RunNewSolutionSingleThreaded(bool showResults = false)
{
	g_countOfCombinations = 0;
	SimpleTimer::SimpleTimer simpleTimer;
	RunNewSolution(showResults);
	std::cout << "Count of combinations " << g_countOfCombinations << std::endl;
}

int main()
{
	//Test();
	//TestFindAllFittings();
	RunNewSolution(7);
	Run(true);
	system("pause");
	return 0;
	RunNewSolution(15);
	Run(true);

	system("pause");
	return 0;
	SimulateInitialization();
	NewSolution::InitializeCombinations();
	//NewSolution::ShowLists();
	NewSolution::FindAllCombinationsForAList(NewSolution::combinations.front());
	system("pause");
	return 0;
	g_countOfCombinations = 0;

	if (
		false
		//!InitializeStreams()
		) {
		std::cout << "File \"input.txt\" doesn't exist. Quiting..." << std::endl;
		return 1;
	}

	if (!InitializeInputData()) {
		std::cout << "Input file is currupted... Quiting..." << std::endl;
		return 1;
	}

	Run(true);
	//RunMultiThreaded();

	//Output();
	//CloseStreams();
	
	system("pause");
    return 0;
}

