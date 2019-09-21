#include "list.h"

int heapInitialization();
void RandomTest(list obj);
using namespace std;
int main()
{
	list obj(128*1024*1024);
	/*obj.printHeapInfo();

	printf( "\nAllocated Blocks = \n");
	void* p1 = obj.memalloc(198 * 16);
	cout << "Pointer 1 = " << p1;
	obj.show(p1);
	void* p2 = obj.memalloc(698 * 16);
	cout << "Pointer 2 = " << p2;
	obj.show(p2);
	obj.memfree(p1);
	void* p3 = obj.memalloc(199 * 16);
	cout << "Pointer 3 = " << p3;
	obj.show(p3);
	obj.memfree(p2);
	void* p4 = obj.memalloc(28 * 16);
	cout << "Pointer 4 = " << p4;
	obj.show(p4);
	printf("\n");
	obj.printFreeList();*/



	RandomTest(obj);
	system("pause");
	return 0;
}

int const MX = (int)1e4 + 5;								// Up to 10,000  allocations/frees
void* pointers[MX];
void RandomTest(list obj)
{
	std::map<void*, int> mapPointers;
	srand((unsigned int)time(0));							// time()    returns time_t which is probably 64-bit int 
															// srand()   wants takes unsigned int which is probably 32-bit int 
	obj.printHeapInfo();
	int ran = 0;
	int allocCount(0), freeCount(0);
	
	Timer calcTime;
	for (int j = 0; j < 25; j++)
	{
		int allocLoop = (rand()) + 1;					// Random variables from 1 to 50 , it can be modified by the user
		for (int i = 0; i < allocLoop; i++)
		{
			ran = ((int)rand() % 2000);
			mapPointers.insert(std::pair<void*, int>(obj.memalloc(ran), 0));
		}

		int sz = mapPointers.size();

		std::map<void*, int>::iterator it = mapPointers.begin();

		int freeLoop = (rand()) + 1;					// Random variables from 1 to 30 , it can be modified by the user
		for (int i = 0; i < (freeLoop%sz) + 1; i++)
		{
			if (it != mapPointers.end())
			{
				obj.memfree(it->first);				// free random pointers
				auto itr = it->first;
				++it;
				mapPointers.erase(itr);
				freeCount++;
			}
		}
		allocCount += allocLoop;
	}
	std::cout << "Heap Remaining = " << obj.heapRemainingGetter()*16 << " Bytes" << '\n';
	printf("\n");
	std::cout << "Allocation Numbers = " << allocCount << '\n';
	std::cout << "Free Numbers = " << freeCount << '\n';
	printf("\n");
	std::cout << "Allocation Fails = " << obj.allocFailsGetter() << '\n';
	std::cout << "Free Fails = " << obj.freeFailsGetter() << '\n';
	std::cout << "Total Fails = " << obj.failsGetter() << '\n';
	printf("\n");
	std::cout << "Internal Fragmentation = " << obj.internalFragmentation() << " Bytes" << '\n';
	std::cout << "External Fragmentation = " << obj.externalFragmentation() << " Bytes" << '\n';
	//std::cout << "Fragmentation Percentage = " << obj.fragmentationPercentage() << "%\n";
	printf("\n");
	printf("RandomTest");
}



int heapInitialization()
{
	int heapSizeType = 1, heapSize;
	std::string sizeType;
	while (1)
	{
		printf("Choose heapsize Type B/KB/MB/GB: ");
		std::cin >> sizeType;

		// convert all chars to lowercase
		std::locale loc;
		std::string sizeTypeLowerCase = "";
		for (auto elem : sizeType)
			sizeTypeLowerCase += std::tolower(elem, loc);

		if (sizeTypeLowerCase == "b")
			break;
		if (sizeTypeLowerCase == "kb")
		{
			heapSizeType = KB;
			break;
		}
		if (sizeTypeLowerCase == "mb")
		{
			heapSizeType = MB;
			break;
		}
		if (sizeTypeLowerCase == "gb")
		{
			heapSizeType = GB;
			break;
		}

		printf("Invalid input\n");
	}
	printf("= ");
	std::cin >> heapSize;

	std::cout << "heapSize = " << (heapSize * heapSizeType) << " in Bytes" << '\n';

	return heapSize * heapSizeType;
}

