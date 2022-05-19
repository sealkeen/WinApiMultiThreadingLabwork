#include "stdafx.h"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <chrono>
#include <queue>
#include <list>

typedef std::vector<int> SolutionIntContainer;
typedef int ReverseIterator;

//#define SHOW_RESULTS

CRITICAL_SECTION cs;
HANDLE sem;
HANDLE* threads;

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


// TODO: clean
namespace NewSolution {

	bool Equal(SolutionIntContainer* left, SolutionIntContainer* right) {
		if (left->size() != right->size())
			return false;

		for (int i = 0; i < (left->size() - 1); i++) {
			if ((*left)[i] != (*right)[i])
				return false;
		}

		return true;
	}

	int Difference(int sum) {
		return g_targetNumber - sum;
	}

	int Difference(SolutionIntContainer& targetList) {
		int sum = 0;
		for (auto it = targetList.rbegin(); it != targetList.rend(); it++) {
			sum += (*it);
		}
		return Difference(sum);
	}

	class CombinationStorage {
	private:
		//int * digit;
		//SolutionCombinationHandler* ancestor;
		//std::list<SolutionIntContainer> descendents;
		std::list<SolutionIntContainer> descendants;
	public:
		// constructors 
		CombinationStorage() {
			descendants = std::list<SolutionIntContainer>();
		}

		// methods
		void DeleteDescendants() {
			descendants.clear();
		}

		bool FindDescendant(SolutionIntContainer* sIC) {
			for (SolutionIntContainer ghost : descendants) {
				if (Equal(&ghost, sIC))
					return true;
			}
			return false;
		}
		void AddDescendant(SolutionIntContainer* combinationNode) {
			if (combinationNode != nullptr)
				descendants.push_back(SolutionIntContainer(*combinationNode));
		}

		SolutionIntContainer* GetNextDescendant() {
			return &descendants.front();
		}
		void DeleteFrontDescendant() {
			descendants.pop_front();
		}

	}; // class CombinationComparator

	class ListHandler {
	private:
		CombinationStorage combinationStorage;
		SolutionIntContainer sourceList;
	public:
		// public fields:
		static std::list<SolutionIntContainer> startingCombinations;

		//constructor:
		ListHandler(SolutionIntContainer& s)
		{
			sourceList = SolutionIntContainer(s);
			//InitializeCombinations();
		}

		//methods:
		void DeleteInnerDescendants()
		{
			combinationStorage.DeleteDescendants();
		}

		void NewDescendantFound(SolutionIntContainer* combination)
		{
			g_countOfCombinations++;
			combinationStorage.AddDescendant(combination);
		}

		void CombinationFound(SolutionIntContainer* combination) {
			g_countOfCombinations++;
		}

		bool RightIsGreater(int left, int right, int offset) {
			if (right > (left + offset))
				return true;
			return false;
		}

		bool RightIsGreaterOrEqual(int left, int right) {
			if (right >= left)
				return true;
			return false;
		}

		int TakeValue(SolutionIntContainer& list, int index) {
			if (index < 0 || index >= list.size())
				return -1;
			return list[index];
		}

		int IncludingIndexDifference(int sum, int reductionOffset)
		{
			return ((g_targetNumber - reductionOffset) - sum);
		}

		//Tested OK 10:47 PM 5/23/2019
		bool CheckOrder(SolutionIntContainer& list, int maxIndex)
		{
			// TODO: optimize 
			int leftI = 0, rightI = 1;
			int sum = list[leftI];
			for (; rightI <= maxIndex; leftI++, rightI++) {
				if (!RightIsGreaterOrEqual(list[leftI], list[rightI]))
					return 1;
				sum += list[rightI];
			}

			int reductionOffset = 0;
			for (int i = (rightI + 1); i < list.size(); i++)
			{
				reductionOffset += list[i];
			}

			return IncludingIndexDifference(sum, reductionOffset);
		}

		//int ReducedIncludingIndexDifference(SolutionIntContainer& list, ReverseIterator rightIndex) {
		//	int sum = 0;
		//	for (int i = 0; i <= rightIndex; i++) {
		//		sum += list[i];
		//	}
		//	int reductionOffset = 0;
		//	for (int i = (rightIndex+1); i < list.size(); i++)
		//	{
		//		reductionOffset += list[i];
		//	}

		//	return IncludingIndexDifference(sum, reductionOffset);
		//}

		int Difference(std::list<int>& targetList) {
			int sum = 0;
			for (auto it = targetList.rbegin(); it != targetList.rend(); it++) {
				sum += (*it);
			}
			return NewSolution::Difference(sum);
		}

		static void ShowList(SolutionIntContainer& list, std::string* additionalContents = nullptr, bool reversed = false)
		{
#ifdef SHOW_RESULTS
			if (reversed) {
				for (auto i = list.rbegin(); i != list.rend(); i++)
				{
					std::cout << (*i) << " ";
				}
			}
			else {
				for (auto i = list.begin(); i != list.end(); i++) {
					std::cout << (*i) << " ";
				}
			}
			if (additionalContents != nullptr) {
				std::cout << " \t " << *additionalContents << " \t ";
			}
			std::cout << std::endl;
#endif
		}

		static void ShowCombination(SolutionIntContainer& container, std::string* additionalContents = nullptr, bool reversed = false) {
#ifdef SHOW_RESULTS
			std::cout << g_countOfCombinations << ") \t";
			ShowList(container, additionalContents, reversed);
#endif
		}

		void Inc(SolutionIntContainer& list, int index, int count = 1) {

			if (index < 0 || index >= list.size())
				return;
			list[index] = list[index] + count;
		}

		void Dec(SolutionIntContainer& list, int index) {
			if (index < 0 || index >= list.size())
				return;
			list[index] = list[index] - 1;
		}

		SolutionIntContainer TryInc(SolutionIntContainer& list, int index)
		{
			SolutionIntContainer resultingList = SolutionIntContainer(list);
			Inc(resultingList, index);
			return resultingList;
		}

		SolutionIntContainer TryDecAndInc(SolutionIntContainer& list, int reductionIndex, int increasingIndex)
		{
			SolutionIntContainer resultingList = SolutionIntContainer(list);
			Dec(resultingList, reductionIndex);
			Inc(resultingList, increasingIndex);
			return resultingList;
		}

		//bool CheckForNewCombination(SolutionIntContainer list, int rightIndex, int leftIndex)
		//{
		//	if ((list[rightIndex] - 1) >= list[leftIndex])
		//		if ((list[leftIndex] + 1) <= list[rightIndex]) {
		//			GhostFound(&list);
		//			ShowCombination(list);
		//			return true;
		//		}
		//	return false;
		//}

		bool RebuildDownstairs(SolutionIntContainer& list, int rightI, int leftI = -1)
		{
			if (leftI == -1)
				leftI = rightI - 1;

			SolutionIntContainer newTryList = TryDecAndInc(list, rightI, leftI);

			if (CheckOrder(newTryList, rightI) > 0)
				return false;

			if (!combinationStorage.FindDescendant(&newTryList)) {
				NewDescendantFound(&newTryList);

				list[rightI] = list[rightI] - 1;
				list[leftI] = list[leftI] + 1;

#ifdef SHOW_RESULTS
				std::string indexesStr = std::string("rightIndex = ") + std::to_string(rightI) +
					+" ; " + std::string("leftIndex = ") + std::to_string(leftI) + std::string(";");

				ShowCombination(list
					, &indexesStr
				);
#endif //SHOW_RESULTS
				return true;
			}
			return false;
		}

		bool GlidePossible(SolutionIntContainer& sIC)
		{
			int first = sIC[0];
			int last = sIC[sIC.size() - 1];
			int offset = 1;

			if (last > (first + offset))
			{
				return true;
			}
			return false;
		}

		int startingLeftLeftIndex;
		bool glode = false;
		int lastRight = 1;
		int lastLeft = 1;

		bool Exists(SolutionIntContainer& list, int minR, int maxL) {
			if (minR == lastRight && maxL == lastLeft)
				return true;
			return false;
		}

		void TryGhostGlideALine(SolutionIntContainer& list, int rightIndex, int leftLeftI,
			int& depthOverall, int currentDepth)
		{
			SolutionIntContainer newTryList = SolutionIntContainer(list);

			while (depthOverall == currentDepth) //(true)
			{
				if (leftLeftI < 0) {
					leftLeftI = startingLeftLeftIndex;
				}

				if (!GlidePossible(newTryList))
					return;
				newTryList = TryDecAndInc(newTryList, rightIndex, leftLeftI);

				if (CheckOrder(newTryList, rightIndex) <= 0) {
					if (
						!combinationStorage.FindDescendant(&newTryList)
						//&&
						//true
						//!Exists(newTryList, newTryList[rightIndex], newTryList[leftLeftI])
						) {
						//NewGhostFound(&list);
						//TODO: erase 2 / restore 1 || erase 1 / restore 2
						NewDescendantFound(&newTryList);
						//g_countOfCombinations++;

						//if(newTryList[rightIndex] < lastRight)
							lastRight = newTryList[rightIndex];
						//if (newTryList[leftLeftI] > lastLeft)
							lastLeft = newTryList[leftLeftI];
						
						ShowCombination(newTryList);
					}

					//CheckForNewCombination(newTryList, rightIndex, leftLeftI);

					TryGhostGlideALine(newTryList, rightIndex, leftLeftI, ++depthOverall, ++currentDepth);
					//--depthOverall;

					// Repeat recursively
					//if (  (leftLeftI >= 1) 
					//	 && (maxLeftPosition <= leftLeftI) 
					//)
					TryGhostGlideALine(newTryList, rightIndex, (leftLeftI - 1),
						(depthOverall = currentDepth),
						/*++*/currentDepth
					);
					//else
					//	return;
				} else {
					//glode = true; //TODO: check if needed
					return;
				}
				leftLeftI--;
			}
			//} else return false;
				//glode = true; //TODO: check if needed
		}

		void ChangeValue(SolutionIntContainer& container, int index, int value) {
			container[index] = value;
		}

		bool CanDecRightAndIncLeft(SolutionIntContainer& list, int rightI, int leftI) {
			return CheckOrder(TryDecAndInc(list, rightI, leftI), rightI) <= 0;
		}

		bool FindNext(SolutionIntContainer& list, int rightI)
		{
			bool result = true;

			// working with previous
			// Find the left Element
			// Check if left element is to the left of right
			int leftI = (rightI - 1);

			// exit if can't shift
			// (exit if left doesn't exist)
			if (leftI < 0)
				return false;

			// TODO: exit condition
			if (!RebuildDownstairs(list, rightI))
				//result = false;
				return false;

			// a third element we are currently working on 
			// it is to the third position to the left 
			// and more if needed 
			int leftLeftI = (leftI - 1);
			if (leftLeftI >= 0)
			{
				//for (int i = 0; i < list[leftI]; i++)
				//{
				//	SolutionIntContainer sIC;
				startingLeftLeftIndex = leftLeftI;
				int depth = 0;

				lastRight = list[rightI];
				lastLeft = 1;
				//TODO: check if works
				combinationStorage.DeleteDescendants();
				TryGhostGlideALine(list, rightI, leftLeftI, depth, depth//, 0
				);
				//}
			}

			// Ghost Glide OK, continue
			return result;
		}

		// Rebuild saving existing right element
		bool RebuildCombination(SolutionIntContainer& list,
			ReverseIterator rightIndex)
		{
			if (rightIndex <= 0)
				return false;
			// strictly more than 0 because we move towards left border
			// and check for the previous (right - 1) element

			int orderDisplacement = CheckOrder(list, rightIndex);

			if (orderDisplacement < 0)
				Inc(list, rightIndex);
			if (orderDisplacement > 0)
				return false;

			int leftI = (rightIndex - 1);

			// Find the changed
			while (FindNext(list, rightIndex)) {

			}
			return true;
		}

		static void InitializeCombinations()
		{
			for (int i = g_targetNumber; i > 1; i--)
			{
				SolutionIntContainer newList = SolutionIntContainer();
				for (int k = 0; k < i; k++) {
					newList.push_back(1);
				}

				startingCombinations.push_back(newList);
			}
		}

		//Debug
		void ShowLists()
		{
			for (auto it1 = startingCombinations.begin(); it1 != startingCombinations.end(); it1++) {
				for (auto it2 = it1->begin(); it2 != it1->end(); it2++) {
					std::cout << (*it2) << " ";
				}
				std::cout << std::endl;
			}
		}

		void FindAllSumsFixedLength()
		{ 
			//std::cout << "Entered Instance Method FindAllSumsFixedLength" << std::endl;
			
			ShowList(sourceList);
			int rightIndex = /*firstRun ?*/ sourceList.size() - 1 /*: iterator - 1*/;
			int left = (rightIndex - 1);
			int difference = g_targetNumber;

			while ((difference = NewSolution::Difference(sourceList)) > 0) {
				sourceList[rightIndex]++;
			}

			NewDescendantFound(&sourceList);
			ShowCombination(sourceList);

			while (
				RebuildCombination(sourceList, rightIndex)
				)
			{
				--rightIndex;
			}
			//std::cout << "Quiting Instance Method FindAllSumsFixedLength" << std::endl;
		}

		static SolutionIntContainer Next()
		{
			EnterCriticalSection(&::cs);
			if (startingCombinations.size() == 0)
				return SolutionIntContainer();
			SolutionIntContainer sourceList = SolutionIntContainer(startingCombinations.front());
			startingCombinations.pop_front();
			LeaveCriticalSection(&::cs);
			return sourceList;
		}

		static void WriteLineThreadBlocking(const char* line, int length = 0)
		{
			EnterCriticalSection(&::cs);
			std::cout << line << std::endl;
			LeaveCriticalSection(&::cs);
		}

		static DWORD WINAPI FindAllSumsFixedLengthMT2(LPVOID pListHandler)
		{
			WriteLineThreadBlocking("Entered thread");
			ListHandler* pLH = (ListHandler*)pListHandler;

			DWORD dwWaitResult;
			BOOL bContinue = TRUE;

			// Try to enter the semaphore gate.

			dwWaitResult = WaitForSingleObject(
				sem,   // handle to semaphore
				INFINITE);           // zero-second time-out interval

			int rightIndex = 0;
			int left = 0;
			int difference = g_targetNumber;

			std::string strSucceded = std::string();
			strSucceded.append("Thread ");
			strSucceded.append(std::to_string(GetCurrentThreadId()));
			strSucceded.append(": wait succeeded\n");

			switch (dwWaitResult)
			{
				// The semaphore object was signaled.
			case WAIT_OBJECT_0:
				// TODO: Perform task
				WriteLineThreadBlocking(strSucceded.c_str());
				bContinue = FALSE;

				// Simulate thread spending time on task
				WriteLineThreadBlocking("Entered Task");
				//std::cout << "Entered Task" << std::endl;

				ShowList(pLH->sourceList);
				rightIndex = /*firstRun ?*/ pLH->sourceList.size() - 1 /*: iterator - 1*/;
				left = (rightIndex - 1);
				
				//TODO: test Added 9/24/2019
				if (rightIndex < 0)
					return FALSE;
				while ((difference = NewSolution::Difference(pLH->sourceList)) > 0) {
					pLH->sourceList[rightIndex]++;
				}

				pLH->NewDescendantFound(&pLH->sourceList);
				ShowCombination(pLH->sourceList);

				while (
					pLH->RebuildCombination(pLH->sourceList, rightIndex)
					)
				{
					--rightIndex;
				}
				pLH->combinationStorage.DeleteDescendants();

				// Release the semaphore when task is finished

				if (!ReleaseSemaphore(
					sem,  // handle to semaphore
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
			delete pLH;
			return TRUE;
		}

		//TODO: Debug and figure out what's wrong here
		static DWORD WINAPI FindAllSumsFixedLengthMT(LPVOID lpParam)
		{
			//std::cout << "Entered FindAllSumsFixedLengthMT" << std::endl;
			// lpParam not used in this example
			UNREFERENCED_PARAMETER(lpParam);

			DWORD dwWaitResult;
			BOOL bContinue = TRUE;

			//std::cout << "Count of combinations : " << startingCombinations.size() << std::endl;
			//while there are combinations unanalyzed
			while (startingCombinations.size() > 0)
			{
				//std::cout << "Waiting for single object" << std::endl;
				// Try to enter the semaphore gate.
				dwWaitResult = WaitForSingleObject(
					::sem,				// handle to semaphore
					INFINITE);			// infinite time-out interval

				SolutionIntContainer next = Next();
				if (next.size() == 0)
					return 1;
				ListHandler lH(next);

				switch (dwWaitResult)
				{
					// The semaphore object was signaled.
				case WAIT_OBJECT_0:
					
					// Perform task
					lH.FindAllSumsFixedLength();
					// TODO: check where we kill ghosts
					lH.DeleteInnerDescendants();

					// The semaphore was nonsignaled, so a time-out occurred.
				case WAIT_TIMEOUT:
					printf("Thread %d: wait timed out\n", GetCurrentThreadId());
					break;
				}
			}
			//std::cout << "Quiting FindAllSumsFixedLengthMT..." << std::endl;
			return TRUE;
		}
	}; // CombinationListHandler
} // New Solution //

namespace CombinationsFinder {

	bool showResults = true;

	/*    arr - array to store the combination
	index - next location in array
	num - given number
	reducedNum - reduced number */
	void findCombinationsUtil( int arr[], int index,
		int num, int reducedNum )
	{
		// Base condition 
		if (reducedNum < 0)
			return;

		// If combination is found, print it 
		if (reducedNum == 0)
		{
			g_countOfCombinations++;
			if (CombinationsFinder::showResults) {
				SolutionIntContainer container(arr, arr + index);
				
				NewSolution::ListHandler::ShowCombination(container, nullptr//, true
					);

				//for (int i = 0; i < index; i++)
				//	std::cout << arr[i] << " ";
				//std::cout << std::endl;
			}
			return;
		}

		// Find the previous number stored in arr[] 
		// It helps in maintaining increasing order 
		int prev = (index == 0) ? 1 : arr[index - 1];

		// note loop starts from previous number 
		// i.e. at array location index - 1 
		for (int k = prev; k <= num; k++) {
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
	void findCombinations(int n, bool showResults = true) {
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

bool InitializeStreams()
{
	try {
		g_inputStream = std::ifstream();
		g_inputStream.open("input.txt");

		g_outputStream = std::ofstream();
		g_outputStream.open("output.txt", std::ios::trunc);

		return true;
	} catch (...) {
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

		if (g_threadCount == -1 || g_targetNumber == -1) {
			return false;
		}
	} catch (...) {
		return false;
	}
	
	return true;
}

void SimulateInitialization(int targetNumber = 10, int threadCount = 2) {
	g_threadCount = threadCount;
	g_targetNumber = targetNumber;
}

std::list<SolutionIntContainer> NewSolution::ListHandler::startingCombinations;

void RunNewSolutionMultiThreaded2() {
	g_countOfCombinations = 0;
	NewSolution::ListHandler::InitializeCombinations();

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
	threads = new HANDLE[NewSolution::ListHandler::startingCombinations.size()];

	for (int i = 0; i < NewSolution::ListHandler::startingCombinations.size(); i++)
	{
		threads[i] = 0;
	}

	int lastThreadID = -1;
	for (auto i = NewSolution::ListHandler::startingCombinations.begin(); i != NewSolution::ListHandler::startingCombinations.end(); i++)
	{
		if (NewSolution::Difference(*i) == 0) {
			g_countOfCombinations++;
			NewSolution::ListHandler::ShowCombination(*i);
			continue;
		}
		++lastThreadID;
		NewSolution::ListHandler* listHandler = new NewSolution::ListHandler(*i);

		// TODO: check where we kill ghosts
		listHandler->DeleteInnerDescendants();
		//listHandler.FindAllSumsFixedLength();

		threads[lastThreadID] = CreateThread(
			NULL,       // default security attributes
			0,          // default stack size
			(LPTHREAD_START_ROUTINE)NewSolution::ListHandler::FindAllSumsFixedLengthMT2,
			(LPVOID)listHandler,       // thread function argument
			0,          // default creation flags
			&ThreadID); // receive thread identifier

		if (threads[lastThreadID] == NULL)
		{
			printf("CreateThread error: %d\n", GetLastError());
			return;
		}

	}

	//for (int i = 0; i < g_threadCount; i++)
	//{
	//	threads[i] = CreateThread(
	//		NULL,       // default security attributes
	//		0,          // default stack size
	//		(LPTHREAD_START_ROUTINE)NewSolution::ListHandler::FindAllSumsFixedLengthMT,
	//		NULL,       // no thread function arguments
	//		0,          // default creation flags
	//		&ThreadID); // receive thread identifier

	//	if (threads[i] == NULL)
	//	{
	//		printf("CreateThread error: %d\n", GetLastError());
	//		return;
	//	}
	//}



	// Wait for all threads to terminate
	//WaitForMultipleObjects(g_threadCount, threads, TRUE, INFINITE);

	WaitForMultipleObjects(lastThreadID + 1, threads, TRUE, INFINITE);

	// Close thread and semaphore handles
	for (int i = 0; i <= lastThreadID; i++)
		CloseHandle(threads[i]);

	delete[] threads;
	CloseHandle(sem);

	// Release resources used by the critical section object.
	DeleteCriticalSection(&cs);

	std::cout << "Count of combinations " << g_countOfCombinations /*+ g_targetNumber-1*/ << std::endl;
	//std::cout << "Count of combinations " << countOfCombinations /*+ g_targetNumber-1*/ << std::endl;
	std::cout << "*** New Solution Execution Completed ***" << std::endl;
}

void RunNewSolutionMultiThreaded( ) {

	g_countOfCombinations = 0;
	NewSolution::ListHandler::InitializeCombinations();
	
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
			(LPTHREAD_START_ROUTINE)NewSolution::ListHandler::FindAllSumsFixedLengthMT,
			NULL,       // no thread function arguments
			0,          // default creation flags
			&ThreadID); // receive thread identifier

		if (threads[i] == NULL)
		{
			printf("CreateThread error: %d\n", GetLastError());
			return;
		}
	}

	// Wait for all threads to terminate
	WaitForMultipleObjects(g_threadCount, threads, TRUE, INFINITE);

	// Close thread and semaphore handles
	for (int i = 0; i < g_threadCount; i++)
		CloseHandle(threads[i]);

	delete[] threads;
	CloseHandle(sem);

	// Release resources used by the critical section object.
	DeleteCriticalSection(&cs);

	std::cout << "Count of combinations " << g_countOfCombinations /*+ g_targetNumber-1*/ << std::endl;
	//std::cout << "Count of combinations " << countOfCombinations /*+ g_targetNumber-1*/ << std::endl;
	std::cout << "*** New Solution Execution Completed ***" << std::endl;
}

void RunNewSolution(bool showResults = false) {
	g_countOfCombinations = 0;
	NewSolution::ListHandler::InitializeCombinations();
	
	for ( auto i = NewSolution::ListHandler::startingCombinations.begin(); i != NewSolution::ListHandler::startingCombinations.end(); i++ )
	{
		if (NewSolution::Difference(*i) == 0) {
			g_countOfCombinations++;
			NewSolution::ListHandler::ShowCombination((*i));
			continue;
		}
		NewSolution::ListHandler listHandler(*i);

		// TODO: check where we kill ghosts
		listHandler.DeleteInnerDescendants();
		listHandler.FindAllSumsFixedLength();
	}

	std::cout << "Count of combinations " << g_countOfCombinations /*+ g_targetNumber-1*/ << std::endl;
	//std::cout << "Count of combinations " << countOfCombinations /*+ g_targetNumber-1*/ << std::endl;
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

void TestCombinationHandler()
{
	NewSolution::CombinationStorage sCH;
	SolutionIntContainer sIC;
	sIC.push_back(1);
	sCH.AddDescendant(&sIC);
	bool truth = sCH.FindDescendant(&sIC);
}

int main()
{
	SimulateInitialization(10, 1);
	//std::cout << "C = " << Soch() << std::endl;

	//Test();
	//return 1;
	//TestFindAllFittings();
	//RunNewSolutionSingleThreaded(true);
	RunNewSolutionMultiThreaded2();
	Run(true);
	//Run_MT2(true);
	system("pause");
	return 0;
	RunNewSolution(15);
	Run(true);

	system("pause");
	return 0;
	//NewSolution::InitializeCombinations();
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

	if ( !InitializeInputData() ) {
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

