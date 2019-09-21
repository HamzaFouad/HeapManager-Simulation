#pragma
#include <iostream>
#include <string>
#include <locale>			// std::locale, std::tolower
#include <assert.h>
#include <stdlib.h>			// int
#include <cmath>
#include <ctime>
#include <chrono>			// std::chrono::time
#include <map>
#include <iterator>
#include <vector>

#define KB 1*1024
#define MB 1*1024*1024
#define GB 1*1024*1024*1024

enum blockStatus { FREE, INUSE };

typedef char chunkelemtype;

class list
{
private:
	long long heapSize;
	struct node;
	typedef node* link;
	int allocFailsCounter, freeFailsCounter, FailsCount;
	long long extSize, heapRemaining, extFragmentation, lastExternal, external;
	bool allocated = false;
	int count = 0;
	void* freeForExt[10000] = { 0 };
	bool extFrag = false;
	std::map<void*, long long> mp;

	struct node
	{
		link adjChunk;
		long long size;
	};

	char* heapStart;
	char* heapEnd;
	char* freeChunkStart;
	link head;
	link tail;
	long long chunkSize;
	list* classPtr;
	/****************************************************/
	int getUnitSize(void);
	long long convertBytesToUnits(long long bytes);
	void* getNext(link Block);
	void setNext(link firstBlock, link secondBlock);
	void* getPrev(link Block);
	void setPrev(link firstBlock, link secondBlock);
	long long getBlockSize(link Block);
	long long getFooterUnits(link Block);
	void setUnits(link Block, long long Units);
	enum blockStatus getStatus(link ptr);
	void setStatus(link Block, enum blockStatus status);
	void* getNextInMem(link Block);
	void* getPrevInMem(link Block);
	void removeFromList(link ptr);
	void splitChunk(link chunckStart, long long totalSize);
	void insertFree(link ptr);
	void* merge(link ptr, link ptr2);
	bool checkFreeList(link Block);
	bool checkInList(link Block);
	bool checkValidBlock(link Block);
	long long computeExtFragmentation();

public:
	list(int heapSize);
	~list();
	void* memalloc(unsigned long long reqSize);
	void memfree(void* ptr);
	double fragmentationPercentage();
	long long internalFragmentation();
	void show(void* p);
	void printHeapInfo();
	void printFreeList();
	long long externalFragmentation() { return extFragmentation; }
	int failsGetter() { return (allocFailsCounter + freeFailsCounter); }
	int allocFailsGetter() { return allocFailsCounter; }
	int freeFailsGetter() { return freeFailsCounter; }
	long long heapRemainingGetter() { return heapRemaining / 16; }






};

struct Timer
{
	std::chrono::time_point<std::chrono::steady_clock> startTime, endTime;
	std::chrono::duration<float> duration;

	Timer()
	{
		startTime = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		endTime = std::chrono::high_resolution_clock::now();
		duration = endTime - startTime;
		float ms = duration.count() * 1000.0f;
		std::cout << " timer = " << ms << "ms\n";
	}

};