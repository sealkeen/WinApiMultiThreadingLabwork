// LabMultiThreading.cpp: ���������� ����� ����� ��� ����������� ����������.
//

#include "stdafx.h"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <chrono>
	
CRITICAL_SECTION cs;

//using namespace std;
std::ifstream g_inputStream;
std::ofstream g_outputStream;

//	������� ������ : ���� input.txt.
//	������ ������ � ����������� ����� T, ���������� ������� �������� ���� ���
//	������� ������.
int g_threadCount;
//	������ ������ � ����������� ����� N(2 <= N <= 100)
int g_minNumber = 2, g_maxNumber = 100;
//	�������� ������ : ���� output.txt, �������� 3 ������

//	������ ������ � ����������� �����, ���������� �������(��� �� �������
//	�����).
//	������ ������ � ����������� ����� N(��� �� ������� �����).
//	������ ������ � ������� ������, ���������� ���������� ���������� �����
//	�� ���������.

//	����� ���������� ���������� ����������.����������, ������������
//	�������� ���������, ��������� �����������(�������� 3 = 2 + 1 � 3 = 1 + 2).
//	����������� ��������� � ����� expr.cpp

std::vector<int> g_possibleCurrentNumbers;

int g_countOfCombinations = 0;

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

	void RemoveLasts()
	{
		try {
			reducedNumbers.pop_back();
			indexes.pop_back();
		}
		catch (...) {

		}
	}

	bool ReturnCondition() {
		// Base condition 
		if (reducedNumbers.back() < 0) {
			RemoveLasts();
			return true;
		}

		// If combination is found, print it 
		if (reducedNumbers.back() == 0) {
			g_countOfCombinations++;

			for (int i = 0; i < indexes.back(); i++)
				std::cout << arr[i] << " ";
			std::cout << std::endl;

			RemoveLasts();
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
			int prev = (indexes.back() == 0) ? 1 : arr[indexes.back() - 1];

			// note loop starts from previous number 
			// i.e. at array location index - 1 
			for (int k = prev; k <= targetNumber; k++)
			{
				// next element of array is k 
				arr[indexes.back()] = k;

				do {

					int prev = (indexes.back() == 0) ? 1 : arr[indexes.back() - 1];
					// call recursively with reduced number 
					indexes.push_back(indexes.back() + 1);
					reducedNumbers.push_back(reducedNumbers.back() - k);

				} while (!ReturnCondition());

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

void Calculate()
{
	//�� g_minNumber �� g_maxNumber
	int maxCountOfDivisions = g_maxNumber - g_minNumber;

	//����� ������ ���
	//for (int g_currentNumber = g_minNumber; g_currentNumber < g_maxNumber; g_currentNumber++) 
	{
		//��������� ������� ���������, ����� ������� �� ����
		g_possibleCurrentNumbers.clear();
		//��������, ��� ����� ����� ��������� �����
		if (CheckForSum())
		{
			//��������, ��� ����� ����� ��� �� ���� �������
			if (!IdenticalPairExistsAlready()) 
			{
				HandleNumbers();
				//���������� ��� � ���������, ���������� ��������� ���� 

				//���������� �������� ���
			}
		}
	}
}


//	�������� ������ : ���� time.txt.�������� ������������ ����� � �����
//	������ ������, � �������������. ���������� ������ ������ ����� ������
//	���������, ��� ����� ������� �������� ������� ������ � �������� �������.


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

	CombinationsFinder::findCombinations(5);
	//CombinationsFinder::findCombinations(50);

	std::cout << "Count of combinations " << g_countOfCombinations << std::endl;

	//std::string timeSpent = simpleTimer.Stop();
	//g_outputStream.write(timeSpent.c_str(), timeSpent.length());
	simpleTimer.~SimpleTimer();

	//CloseStreams();
	system("pause");
    return 0;
}

