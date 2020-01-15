#include <windows.h>
#include <deque>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <conio.h>
#include <time.h>
#include <iostream>

int numThreads;
std::string filePath;
std::string filePath_to_save;
static std::vector<std::string> **sorted_string_arrs;
static int Synch = 0;
static int countEl;
static int isAdditing;
std::vector<std::string> *vect;
std::vector<std::string> Arr;


class Arr_id
{
public:
	std::vector<std::string> vector;
	Arr_id(int id, std::vector<std::string> vector)
	{
		this->id = id;
		this->vector = vector;
	}

	int getId()
	{
		return this->id;
	}

	std::vector<std::string>* getVector()
	{
		return &(this->vector);
	}

private:
	int id;
};


Arr_id* arr_id;

HANDLE hMutex;

DWORD WINAPI sort_func(LPVOID lParam)
{
	Arr_id* arr_id = ((Arr_id*)lParam);


	
	int id = arr_id->getId();

	std::vector<std::string>* vector = arr_id->getVector();
	std::sort((*vector).begin(), (*vector).end());

	sorted_string_arrs[id] = vector;

	return 0;
}




class Element
{
public:

	Element(std::vector<std::string> str_Arr)
	
	{
		this->str_Arr = str_Arr;
	}

	std::vector<std::string>* getstringArr()
	{
		return &this->str_Arr;
	}



private:
	std::vector<std::string> str_Arr;

};

class Queue
{
public:

	Queue()
	{
		isAdditing = 0;
		this->countEl = 0;
	}

	int addToQueue(Element element)
	{
		WaitForSingleObject(hMutex, INFINITE);  // останавливает выполнения программы до тех пор пока объект , идентификатор которого передан в функцию ,не окажется в "сигнальном" состоянии
		
		while (isAdditing)
			Sleep(100);
		
		isAdditing = 1;
		Arr_elements.push_back(element);       // добавление в конец
		this->countEl += 1;
		isAdditing = 0;
		ReleaseMutex(hMutex);                  // освобождение объекта 
		
		return this->countEl;
	}

	int getCountEl()
	{
		return this->countEl;
	}

	Element getElement()
	{
		while (isAdditing)
			Sleep(10);
		isAdditing = 1;
	
		Element Temp = Arr_elements.back();   // Возвращает ссылку на последний элемент в векторе
		Arr_elements.pop_back();			  
		this->countEl--;
		isAdditing = 0;
		return Temp;
		
	}

	
private:
	std::vector<Element> Arr_elements;
	int countEl;
};


class ThreadsApi
{
public:
	HANDLE * Threads;

	ThreadsApi(int numThreads)
	{
		this->Threads = new HANDLE[numThreads];
		this->numThreads = numThreads;
	}

	int getFree()
	{
		bool isFree = false;
		int i;
		while (isFree)
		{
			this->numActiv = 0;
			for (i = 0; i < this->numThreads; i++)
			{
				if  ((this->Threads[i] != NULL) || (WaitForSingleObject(this->Threads[i], 5)))
					isFree = true;
				else
					this->numActiv++;
			}
			if (isFree)
				return i;
		}
	}

	int getNumActiv()
	{
		return this->numActiv;
	}

private:

	int numActiv = 0;
	int numThreads;
};


class QueueWorker
{
public:

	QueueWorker()
	{
		
	}

	QueueWorker(int numThreads, Queue queue)
	{
		this->queue = queue;
		this->numThreads = numThreads;	
		this->Threads = new ThreadsApi(numThreads);
		this->countEl = queue.getCountEl();
	}



	void startQueue()
	{
		int index = 0;
		int thread_state;
		


		while (this->countEl-- > 0)
		{
			Element element = this->queue.getElement();

			arr_id = new Arr_id(index, *element.getstringArr());


			this->Threads->Threads[index++] = CreateThread(NULL,
				0,
				(LPTHREAD_START_ROUTINE) sort_func,
				arr_id,
				0, 
				NULL);
		

		}
		int mes = waitForAll();
	}

	int waitForAll()
	{
		return WaitForMultipleObjects(numThreads, (const HANDLE*)this->Threads->Threads,
			TRUE, INFINITE);
	}

private:
	ThreadsApi* Threads;
	Queue queue;
	int numThreads;
	int countEl;
};


class Sort
{

public:
	Sort()
	{

	}

	Sort(int numThreads, std::string filePath, std::string filePath_to_save)
	{
		this->filePath = filePath;
		this->readFile(filePath);
		this->filePath_to_save = filePath_to_save;
		sorted_string_arrs = new std::vector<std::string>*[numThreads];
		this->numThreads = numThreads;
		this->makeQueue();
	}



	void startSorting()
	{
		this->queueWorker = new QueueWorker(this->numThreads, this->queue);
		this->queueWorker->startQueue();
		this->unionSortesStrings();
		this->sorted_string_vector = this->merge(sorted_string_arrs, numThreads);
		this->saveToFile(this->filePath_to_save);
		return;
	}


private:



	std::string filePath_to_save;
	std::string filePath;
	QueueWorker* queueWorker;
	std::vector<std::string> string_vector;
	std::vector<std::string> sorted_string_vector;
	int numThreads;
	Queue queue;


	std::vector<std::string> merge(std::vector<std::string>** Arr, int num)
	{
		int arr_len = 0; int key; int Num_Min = 0;
		for (int i = 0; i < num; i++)
		{
			arr_len += (*(Arr[i])).size();
		}

		std::vector<std::string> Arr_merged(arr_len);
		int* indexes = new int[num];
		for (int i = 0; i < num; i++)
			indexes[i] = 0;
		std::string Min;
		
		for (int i = 0; i < arr_len; i++)
		{
			for (Num_Min = 0; Num_Min < num; Num_Min++)
			{
				if (indexes[Num_Min] < (*Arr[Num_Min]).size())
				{
					Min = (*Arr[Num_Min])[indexes[Num_Min]];
					break;
				}
			}
			for (key = 0; key < num; key++)
			{
				if ((indexes[key] < (*Arr[key]).size()) && (Min > (*Arr[key])[indexes[key]]))
				{
					Num_Min = key;
				}
			}
			Arr_merged[i] = (*Arr[Num_Min])[indexes[Num_Min]];
			indexes[Num_Min]++;
		}
		return Arr_merged;
	}

	void readFile(std::string filePath)
	{
		std::ifstream file;
		file.open(filePath);
		if (!file.is_open() || !file.good())
			return;
		std::string line;
		while (std::getline(file, line))
			this->string_vector.push_back(line);
		return;
	}

	void makeQueue()
	{
		int len = this->string_vector.size() / this->numThreads;
		std::vector<std::vector<std::string>> Arr;
		for (int i = 0; i < this->numThreads - 1; i++)
		{
			Arr.push_back(std::vector<std::string>(this->string_vector.begin() + i * len, 
				this->string_vector.begin() + (i + 1) * len));
		}


		Arr.push_back(std::vector<std::string>(this->string_vector.begin() + (numThreads-1) * len,
						this->string_vector.end()));
		for (int i = 0; i < this->numThreads; i++)
		{
			countEl++;
			this->queue.addToQueue(Element(Arr[i]));
		}
	}

	void unionSortesStrings()
	{
		int ArrLen = 0;
		for (int i = 0; i < numThreads; i++)
			ArrLen += (*sorted_string_arrs[i]).size();

		this->sorted_string_vector.reserve(ArrLen);
		
		for (int i = 0; i < numThreads; i++)
		{

			this->sorted_string_vector.insert((this->sorted_string_vector).end(),
				(*sorted_string_arrs[i]).begin(),
				(*sorted_string_arrs[i]).end());
		}
	}

	int saveToFile(std::string filePath_to_save)
	{
		std::ofstream file;
		file.open(filePath_to_save);
		if (!file.is_open())
		{
			return -1;
		}
		for (int i = 0; i < this->sorted_string_vector.size(); i++)
		{
			
			file << this->sorted_string_vector[i] << "\n";
		}

		return 0;
	}
};



/*void currentDateTime() {
	
	SYSTEMTIME lpSysT;
	GetSystemTime(&lpSysT);


	printf("%i-%i-%i %i:%i:%i:%i\n", lpSysT.wYear, lpSysT.wMonth,
		lpSysT.wDay, lpSysT.wHour, lpSysT.wMinute, lpSysT.wSecond, lpSysT.wMilliseconds);
}*/

int main()
{

	hMutex = CreateMutex(NULL, FALSE, NULL);
	numThreads = 10;
	filePath = "D:\\TEST_SORT.txt";
	filePath_to_save = "D:\\1.txt";
	Sort Sorter(numThreads, filePath, filePath_to_save);


   
	
	Sorter.startSorting();
	printf("File sorted\n");

	

	_getch();
}

