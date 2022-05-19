#include "stdafx.h"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <chrono>
#include <queue>

typedef int ReverseIterator;

//#define SHOW_RESULTS

CRITICAL_SECTION cs;
HANDLE sem;
HANDLE* threads;

//using namespace std;
std::ifstream g_inputStream;
std::ofstream g_outputStream;

int g_threadCount;
int g_countOfCombinations = 0;
int g_targetNumber = 0;
std::string g_millisecondsSpentOutputStr;

namespace Set {
	class ContainerSet {
	public:
		bool _empty;
		int _size;
		int* _collection;
		int _lastIndex;
		ContainerSet* next;
		ContainerSet()
		{
			_lastIndex = 0;
			_size = 0;
			_collection = nullptr;
			_empty = true;
		}

		ContainerSet(const ContainerSet& source)
		{
			Initialize(source);
		}

		void Initialize(const ContainerSet& source)
		{
			_lastIndex = 0;
			_size = source._size;
			_collection = new int[_size];

			for (int i = 0; i < _size; i++) {
				_collection[i] = source[i];
			}
			_empty = false;
		}

		ContainerSet(int size) 
		{
			_lastIndex = 0;
			_collection = new int[size];
			_size = size;
			_empty = false;
		}
		
		~ContainerSet() 
		{
			if (_empty == true)
				return;
			else {
				if (_collection) {
					delete _collection;
					_collection = nullptr;
					_empty = true;
				}
			}
		}

		int operator[] (int i) const {
			if (_collection == nullptr)
				return -1;
			return _collection[i];
		}

		int& operator[](int i) {
			if (_collection == nullptr)
				return _collection[0];
			return _collection[i];
		}

		bool operator<(const ContainerSet& left) 
		{
			return left._collection < this->_collection;
		}

		int size() { if (this == nullptr) return 0; return _size; }

		bool Equal(ContainerSet* set) {
			if (_collection == nullptr || _size < 0 )
				return false;
			
			for (int i = 0; i < set->size(); i++)
			{
				if (set->_collection[i] != this->_collection[i])
					return false;
			}
			if(set->size() == this->size())
				return true;
			return false;
		}

		int* rbegin() { return &_collection[_size - 1]; }
		int* rend() { return &_collection[0]; }
		int* begin() { return &_collection[0]; }
		int* end() { return &_collection[_size - 1]; }
	};


	class DynamicListOfSets {
		public:
			bool _empty;
			ContainerSet* _last;
			ContainerSet* _first;
			DynamicListOfSets()
			{
				Initialize(this);
			}
			static void Initialize(DynamicListOfSets* list) {

				list->_first = nullptr;
				list->_last = nullptr;
			}
			ContainerSet* back() {
				return _last;
			}
			bool HasEqualMember(ContainerSet* member)
			{
				if (_first == nullptr)
				{
					return false;
				}
				ContainerSet* current = _first;
				do {
					//do stuff
					if (current->Equal(member))
						return true;

					current = current->next;
				} while (current->next != nullptr);
				return false;
			}

			ContainerSet* front() 
			{
				return _first;
			}
			void push_back(ContainerSet* set)
			{
				if (set == nullptr)
					return;

				set->next = nullptr;
				if (_last == nullptr) {
					_first = set; _last = set;
				} else {
					_last->next = set;
					_last = set;
				}
				_empty = false;
			}

			void clear() {
				ContainerSet* current = _first;
				if (current == nullptr) {
					_empty = true;
					return;
				}
				ContainerSet* next = current->next;

				do {
					//extract next from current
					next = current->next;
					//delete current
					delete current;
					//move next 
					current = next;
					//if next exist
				} while (next != nullptr);

				_empty = true;
			}
			void pop_front() {
				if (_first != nullptr)
				{
					auto next = _first->next;
					delete _first;
					if (next != nullptr)
						_first = next;
				}
			}
			~DynamicListOfSets()
			{
				_empty = true;
				EnterCriticalSection(&cs);
				//std::cout << "Destroyed" << std::endl;
				LeaveCriticalSection(&cs);
			}
	};

	class ListOfSets {
	public:
		//data fields
		ContainerSet** _collection;
		int _size;
		int _pointer;

		ContainerSet* operator[] (int i) const {
			if (_collection == nullptr)
				return nullptr;
			return _collection[i];
		}

		//ContainerSet& operator[](int i) {
		//	if (_collection == nullptr)
		//		return *_collection[0];
		//	return *_collection[i];
		//}

		//constructors
		ListOfSets()
		{
			_size = 0;
			_collection = nullptr;
			_pointer = -1;
		}

		ListOfSets(int size)
		{
			Initialize(this, size);
		}

		static void Initialize(ListOfSets* list, int size)
		{
			list->_size = size;
			list->_collection = new ContainerSet*[size];
			for (int i = 0; i < size; i++)
			{
				list->_collection[i] = new ContainerSet(size);
			}
			list->_pointer = -1;
		}

		//methods
		int size()
		{
			return _size;
		}

		void push_back(ContainerSet* list)
		{
			if (!IsFull())
			{
				if (list == nullptr)
				{
					//TODO: check and correct here
					throw 0;
					return;
				}
				_pointer++;
				_collection[_pointer] = list;
			}
		}

		ContainerSet* next(int index)
		{
			if ( (index+1) >= _size)
				return nullptr;
			return _collection[index + 1];
		}

		ContainerSet* front()
		{
			if(_collection != nullptr)
				return _collection[0];
			return nullptr;
		}

		ContainerSet* pop_back()
		{
			if (!HasValues())
				return nullptr;
			else {
				if(_pointer-1 >= 0)
					return _collection[_pointer--];
				return false;
			}
		}

		ContainerSet* pop_front()
		{
			if (!HasValues())
				return nullptr;
			else {
				if (_pointer >= 0)
					return _collection[0];
				return false;
			}
		}

		bool IsFull()
		{
			return (_pointer + 1) >= _size;
		}
		bool HasValues()
		{
			if (_collection == nullptr || _size <= 0)
				return false;
			return true;
		}

		ContainerSet* rbegin()
		{
			if (_collection == nullptr || _collection[_size - 1] == nullptr) 
				return nullptr;
			return _collection[_size - 1];
		}
		ContainerSet* rend()
		{
			if (_collection == nullptr || _collection[0] == nullptr)
				return nullptr;
			return _collection[0];
		}
		ContainerSet* begin()
		{
			if (_collection == nullptr || _collection[0] == nullptr)
				return nullptr;
			return _collection[0];
		}
		ContainerSet* end()
		{
			if (_collection == nullptr || _collection[_size - 1] == nullptr)
				return nullptr;
			return _collection[_size - 1];
		}

		void clear()
		{
			if (_collection == nullptr)
				return;
			if (_collection[0]) {
				for (int cSI = 0; cSI < _size; cSI++)
				{
					if (_collection[cSI] != nullptr)
					{
						delete _collection[cSI];
						_collection[cSI] = new Set::ContainerSet();
					}
				}
			}
			ListOfSets();
		}

		~ListOfSets() {
			clear();
		}
	};
}

typedef Set::ContainerSet SolutionIntContainer;

//typedef std::vector<int> SolutionIntContainer;

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
		for (auto it = targetList.rbegin(); it >= targetList.rend(); it--) {
			sum += (*it);
		}
		return Difference(sum);
	}

	class CombinationStorage {
	private:
		Set::DynamicListOfSets descendants;
	public:
		// constructors 
		CombinationStorage(int size) {
			Set::DynamicListOfSets::Initialize(&descendants);
		}

		// methods
		void DeleteDescendants() {
			//descendants.clear();
		}

		bool FindDescendant(SolutionIntContainer* sIC) {

			auto it = descendants.front();
			while (it != nullptr) {
				if (Equal(it, sIC))
					return true;
				it = it->next;
			} 
				
			return false;
		}
		void AddDescendant(SolutionIntContainer* combinationNode) {
			if (combinationNode != nullptr)
				descendants.push_back(new SolutionIntContainer(*combinationNode));
		}

		SolutionIntContainer* GetNextDescendant() {
			return descendants.front();
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
		static Set::ListOfSets startingCombinations;

		//constructor:
		ListHandler(SolutionIntContainer* s, int size) : combinationStorage(size)
		{
			sourceList.Initialize(*s);
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

		static void ShowList(SolutionIntContainer& list, std::string* additionalContents = nullptr, bool reversed = false)
		{
#ifdef SHOW_RESULTS
			if (reversed) {
				for (auto i = list.rbegin(); i >= list.rend(); i--)
				{
					std::cout << (*i) << " ";
				}
			}
			else {
				for (auto i = list.begin(); 
					i <= list.end(); 
					i++) {
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

		void TryDecAndInc(SolutionIntContainer& newList, SolutionIntContainer& oldList, int reductionIndex, int increasingIndex)
		{
			newList.Initialize(oldList);
			Dec(newList, reductionIndex);
			Inc(newList, increasingIndex);
		}		
		
		SolutionIntContainer& TryDecAndInc(SolutionIntContainer& const newList, int reductionIndex, int increasingIndex)
		{
			Dec(newList, reductionIndex);
			Inc(newList, increasingIndex);
			return newList;
		}

		bool RebuildDownstairs(SolutionIntContainer& list, int rightI, int leftI = -1)
		{
			if (leftI == -1)
				leftI = rightI - 1;

			SolutionIntContainer newTryList;
			TryDecAndInc(newTryList, list, rightI, leftI);

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
			SolutionIntContainer newTryList(list);

			while (depthOverall == currentDepth) //(true)
			{
				if (leftLeftI < 0) {
					leftLeftI = startingLeftLeftIndex;
				}

				if (!GlidePossible(newTryList))
					return;
				TryDecAndInc(newTryList, rightIndex, leftLeftI);

				if (CheckOrder(newTryList, rightIndex) <= 0) {
					if (
						!combinationStorage.FindDescendant(&newTryList)
						) {
						NewDescendantFound(&newTryList);

						//if(newTryList[rightIndex] < lastRight)
							lastRight = newTryList[rightIndex];
						//if (newTryList[leftLeftI] > lastLeft)
							lastLeft = newTryList[leftLeftI];
						
						ShowCombination(newTryList);
					}

					TryGhostGlideALine(newTryList, rightIndex, leftLeftI, ++depthOverall, ++currentDepth);

					// Repeat recursively
					TryGhostGlideALine(newTryList, rightIndex, (leftLeftI - 1),
						(depthOverall = currentDepth),
						/*++*/currentDepth
					);
				} else {
					return;
				}
				leftLeftI--;
			}
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
				return false;

			// a third element we are currently working on 
			// it is to the third position to the left 
			// and more if needed 
			int leftLeftI = (leftI - 1);
			if (leftLeftI >= 0)
			{
				startingLeftLeftIndex = leftLeftI;
				int depth = 0;

				lastRight = list[rightI];
				lastLeft = 1;
				//TODO: check if works
				combinationStorage.DeleteDescendants();
				TryGhostGlideALine(list, rightI, leftLeftI, depth, depth//, 0
				);
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

		static void InitializeCombinations( int count )
		{
			Set::ListOfSets::Initialize(&startingCombinations, count-1);
			for (int i = count; i > 1; i--)
			{
				SolutionIntContainer* newList = new SolutionIntContainer(i);
				for (int k = 0; k < i; k++) {
					(*newList)[k] = 1;
				}

				startingCombinations.push_back(newList);
			}
		}

		//Debug
		//void ShowLists()
		//{
		//	for (auto it1 = startingCombinations.begin(); it1 <= startingCombinations.end(); it1++) {
		//		for (auto it2 = it1->begin(); it2 <= it1->end(); it2++) {
		//			std::cout << (*it2) << " ";
		//		}
		//		std::cout << std::endl;
		//	}
		//}

		void FindAllSumsFixedLength()
		{ 
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
		}

		static SolutionIntContainer Next()
		{
			EnterCriticalSection(&::cs);
			if (startingCombinations.size() == 0)
				return SolutionIntContainer(0);

			SolutionIntContainer* sourceList = startingCombinations.pop_front();

			LeaveCriticalSection(&::cs);
			return *sourceList;
		}

		static void WriteLineThreadBlocking(const char* line, int length = 0)
		{
			EnterCriticalSection(&::cs);
			std::cout << line << std::endl;
			LeaveCriticalSection(&::cs);
		}

		static DWORD WINAPI FindAllSumsFixedLengthMT(LPVOID pListHandler)
		{
			//WriteLineThreadBlocking("Entered thread");
			ListHandler* pLH = (ListHandler*)pListHandler;

			DWORD dwWaitResult;
			BOOL bContinue = TRUE;

			// Try to enter the semaphore gate.

			dwWaitResult = WaitForSingleObject(
				sem,		// handle to semaphore
				INFINITE);  // zero-second time-out interval

			int rightIndex = 0;
			int left = 0;
			int difference = g_targetNumber;

			switch (dwWaitResult)
			{
				// The semaphore object was signaled.
			case WAIT_OBJECT_0:
				//// TODO: Perform task
				bContinue = FALSE;

				//ShowList(pLH->sourceList);
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
					//printf("ReleaseSemaphore error: %d\n", GetLastError());
				}
				break;

				// The semaphore was nonsignaled, so a time-out occurred.
			case WAIT_TIMEOUT:
				//printf("Thread %d: wait timed out\n", GetCurrentThreadId());
				break;
			}
			delete pLH;
			return TRUE;
		}

	}; // CombinationListHandler
} // New Solution //

Set::ListOfSets NewSolution::ListHandler::startingCombinations;

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

		g_threadCount = -1;
		g_targetNumber = -1;

		// Read count of threads from the file
		std::string tempLine;
		std::getline(g_inputStream, tempLine);
		g_threadCount = std::stoi(tempLine);

		// Read the target number from the file
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

void RunNewSolutionMultiThreaded() {

	SimpleTimer::SimpleTimer simpleTimer = SimpleTimer::SimpleTimer();
	g_countOfCombinations = 0;
	NewSolution::ListHandler::InitializeCombinations(g_targetNumber);

	for (int i = 0
		; i < NewSolution::ListHandler::startingCombinations.size(); 
		i++)
	{
		NewSolution::ListHandler::ShowList(*NewSolution::ListHandler::startingCombinations[i]);
	}

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

	int lastThreadID = -1; int i = 0;

	for (int i = 0; 
		i < NewSolution::ListHandler::startingCombinations.size(); 
		i++
		)
	{
		auto it = NewSolution::ListHandler::startingCombinations._collection[i];
		if (NewSolution::Difference(*it) == 0) {
			g_countOfCombinations++;
			//NewSolution::ListHandler::ShowCombination(*it);
			continue;
		}
		++lastThreadID;
		NewSolution::ListHandler* listHandler = new NewSolution::ListHandler(it, it->size());

		listHandler->DeleteInnerDescendants();

		//listHandler->FindAllSumsFixedLength();

		threads[lastThreadID] = CreateThread(
			NULL,       // default security attributes
			0,          // default stack size
			(LPTHREAD_START_ROUTINE)NewSolution::ListHandler::FindAllSumsFixedLengthMT,
			(LPVOID)listHandler,       // thread function argument
			0,          // default creation flags
			&ThreadID); // receive thread identifier

		if (threads[lastThreadID] == NULL)
		{
			printf("CreateThread error: %d\n", GetLastError());
			return;
		}
	}

	// Wait for all threads to terminate
	WaitForMultipleObjects(lastThreadID + 1, threads, TRUE, INFINITE);

	// Close thread and semaphore handles
	for (int i = 0; i <= lastThreadID; i++)
		CloseHandle(threads[i]);

	delete[] threads;
	CloseHandle(sem);
	
	// Release resources used by the critical section object.
	DeleteCriticalSection(&cs);

	//NewSolution::ListHandler::startingCombinations.~ListOfSets();

	std::cout << "Count of combinations " << g_countOfCombinations /*+ g_targetNumber-1*/ << std::endl;
	//std::cout << "Count of combinations " << countOfCombinations /*+ g_targetNumber-1*/ << std::endl;
	std::cout << "*** New Solution Execution Completed ***" << std::endl;

	// Counting time spent
	g_millisecondsSpentOutputStr = simpleTimer.StopMilliseconds();
	std::cout << "Time spent: " << g_millisecondsSpentOutputStr << std::endl;
}

void RunNewSolution(bool showResults = false) {
	g_countOfCombinations = 0;
	NewSolution::ListHandler::InitializeCombinations(g_targetNumber);
	
	//for ( auto i = NewSolution::ListHandler::startingCombinations.begin(); i != NewSolution::ListHandler::startingCombinations.end(); i++ )
	//{
	//	if (NewSolution::Difference(*i) == 0) {
	//		g_countOfCombinations++;
	//		NewSolution::ListHandler::ShowCombination((*i));
	//		continue;
	//	}
	//	NewSolution::ListHandler listHandler(*i);

	//	// TODO: check where we kill ghosts
	//	listHandler.DeleteInnerDescendants();
	//	listHandler.FindAllSumsFixedLength();
	//}

	std::cout << "Count of combinations " << g_countOfCombinations /*+ g_targetNumber-1*/ << std::endl;
	//std::cout << "Count of combinations " << countOfCombinations /*+ g_targetNumber-1*/ << std::endl;
	std::cout << "*** New Solution Execution Completed ***" << std::endl;
}

int testSet()
{
	Set::ContainerSet set(5);

	set[3] = 0;
	for (int i = 0; i < set.size(); i++) {
		std::cout << set[i] << "\n";
	}
	std::cout << set[3];

	set.~ContainerSet();
	set.~ContainerSet();

	system("pause");
	return 1;
}

//Set::ContainerSet WorkWithSet(Set::ContainerSet set) {
//	Set::ContainerSet st = Set::ContainerSet();
//}

int main()
{
	g_countOfCombinations = 0;

	if (
		!InitializeStreams()
	) {
		std::cout << "File \"input.txt\" doesn't exist. Quiting..." << std::endl;
		system("pause");
		return 1;
	}

	if ( !InitializeInputData() ) {
		std::cout << "Input file is currupted... Quiting..." << std::endl;
		system("pause");
		return 1;
	}

	RunNewSolutionMultiThreaded();

	Output();
	CloseStreams();
	system("pause");
	
    return 0;
}

