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
		std::list<SolutionIntContainer> ghostDescendants;
	public:
		// constructors 
		CombinationStorage() {
			ghostDescendants = std::list<SolutionIntContainer>();
		}

		// methods
		void KillGhosts() {
			ghostDescendants.clear();
		}

		bool FindGhost(SolutionIntContainer* sIC) {
			for (SolutionIntContainer ghost : ghostDescendants) {
				if (Equal(&ghost, sIC))
					return true;
			}
			return false;
		}
		void AddGhost(SolutionIntContainer* combinationNode) {
			if (combinationNode != nullptr)
				ghostDescendants.push_back(SolutionIntContainer(*combinationNode));
		}

		SolutionIntContainer* GetNextGhost() {
			return &ghostDescendants.front();
		}
		void DispellFrontGhost() {
			ghostDescendants.pop_front();
		}

	}; // class CombinationComparator

	class ListHandler {
	private:
		CombinationStorage combinationStorage;
	public:
		// public fields:
		static std::list<SolutionIntContainer> startingCombinations;

		//constructor:
		ListHandler()
		{
			//InitializeCombinations();
		}

		//methods:
		void KillInnerGhosts()
		{
			combinationStorage.KillGhosts();
		}

		void NewGhostFound(SolutionIntContainer* combination)
		{
			g_countOfCombinations++;
			combinationStorage.AddGhost(combination);
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

			if (!combinationStorage.FindGhost(&newTryList)) {
				NewGhostFound(&newTryList);

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

					if (!combinationStorage.FindGhost(&newTryList)) {
						//NewGhostFound(&list);
						NewGhostFound(&newTryList);
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
				}
				else {
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

		void FindAllSumsFixedLength(SolutionIntContainer list)
		{
			int rightIndex = /*firstRun ?*/ list.size() - 1 /*: iterator - 1*/;

			int left = (rightIndex - 1);

			int difference = g_targetNumber;

			while ((difference = NewSolution::Difference(list)) > 0) {
				list[rightIndex]++;
			}

			NewGhostFound(&list);
			ShowCombination(list);

			//bool razmazano = false;
			while (
				RebuildCombination(list, rightIndex)
				)
			{
				--rightIndex;
			}
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

void SimulateInitialization() {
	g_threadCount = 2;
	g_targetNumber = 10;
}

void RunMultiThreaded( bool showResults = false ) {
	SimpleTimer::SimpleTimer simpleTimer = SimpleTimer::SimpleTimer();
	//simpleTimer = SimpleTimer::SimpleTimer();
	CombinationsFinder::findCombinations( g_targetNumber, showResults );
	std::cout << "Count of combinations " << g_countOfCombinations << std::endl;
	g_millisecondsSpentOutputStr = simpleTimer.StopMilliseconds();
	//std::string timeSpent = simpleTimer.Stop();
	//g_outputStream.write(timeSpent.c_str(), timeSpent.length());
}

std::list<SolutionIntContainer> NewSolution::ListHandler::startingCombinations;

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
		NewSolution::ListHandler listHandler;

		// TODO: check where we kill ghosts
		listHandler.KillInnerGhosts();

		listHandler.FindAllSumsFixedLength(
			*i
		);
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
	sCH.AddGhost(&sIC);
	bool truth = sCH.FindGhost(&sIC);
}

int main()
{
 	g_targetNumber = 11;
	//std::cout << "C = " << Soch() << std::endl;

	//Test();
	//return 1;
	//TestFindAllFittings();
	RunNewSolutionSingleThreaded(true);
	
	Run(true);
	//Run_MT2(true);
	system("pause");
	return 0;
	RunNewSolution(15);
	Run(true);

	system("pause");
	return 0;
	SimulateInitialization();
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

