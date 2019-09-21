#include "list.h"

list::list(int heapSize)
{
	heapStart = new char[heapSize];
	heapEnd = heapStart + (heapSize);
	heapRemaining = heapEnd - heapStart;
	freeChunkStart = heapStart; // a pointer to the big free chunk of the heap
	head = NULL;
	tail = NULL;
	chunkSize = convertBytesToUnits(heapEnd - heapStart);
	extFragmentation = 0;
}
/*..............................................................*/
list::~list()
{
	delete classPtr;
}
/*..............................................................*/
int list::getUnitSize(void)
// Returns the size of struct (16 bytes)
{
	return sizeof(struct node);
}
/*..............................................................*/
long long list::convertBytesToUnits(long long bytes)
// Convert bytes to units
{
	return (bytes >= 1) ? (((bytes - 1) / (long long)getUnitSize()) + 1) : 0;
}
/*..............................................................*/
void* list::getNext(link Block)
// Return Block's next block in the free linked list
{
	if (Block != NULL)
		return (void*)(Block + getBlockSize(Block) - 1)->adjChunk;
	else
		return NULL;
}
/*..............................................................*/
void list::setNext(link firstBlock, link secondBlock)
{
	//if (firstBlock >= (link)heapEnd || secondBlock >= (link)heapEnd)
	//	return;
	if (firstBlock != NULL && secondBlock != NULL)
	{
		(firstBlock + getBlockSize(firstBlock) - 1)->adjChunk = secondBlock;
		secondBlock->adjChunk = firstBlock;
	}
	else if (firstBlock != NULL && secondBlock == NULL)
	{
		(firstBlock + getBlockSize(firstBlock) - 1)->adjChunk = NULL;
	}
	else if (firstBlock == NULL && secondBlock != NULL)
		secondBlock->adjChunk = NULL;
}
/*..............................................................*/
void* list::getPrev(link Block)
// Return Block's previous block in the free linked list
{
	if (Block != NULL)
		return (void*)Block->adjChunk;
	else
		return NULL;
}
/*..............................................................*/
void list::setPrev(link firstBlock, link secondBlock)
{

	if (firstBlock != NULL && secondBlock != NULL)
	{
		(secondBlock + getBlockSize(secondBlock) - 1)->adjChunk = firstBlock;
		firstBlock->adjChunk = secondBlock;
	}
	else if (firstBlock != NULL && secondBlock == NULL)
	{
		firstBlock->adjChunk = NULL;
	}

	else if (firstBlock == NULL && secondBlock != NULL)
		(secondBlock + getBlockSize(secondBlock) - 1)->adjChunk = NULL;
}
/*..............................................................*/
long long list::getBlockSize(link Block)
// Return Header's units
{
	if (Block != NULL)
		return ((Block->size) >> 1);
	else
		return NULL;
}
/*..............................................................*/
long long list::getFooterUnits(link Block)
// Return Footer's units
{
	if (Block != NULL)
		return ((Block + getBlockSize(Block) - 1)->size);
	else
		return NULL;
}
/*..............................................................*/
void list::setUnits(link Block, long long Units)
// set Block's header and footer units to Units
{
	if (Block != NULL)
	{
		// Set header's units
		Block->size = Block->size & 1U;
		Block->size |= (Units << 1);

		// Set footer's units
		(Block + getBlockSize(Block) - 1)->size = (long long)Units;
	}
}
/*..............................................................*/
enum blockStatus list::getStatus(link Block)
	// Return the Block status
{
	assert(Block != NULL);
	return (enum blockStatus)(Block->size & 1U);

}
/*..............................................................*/
void list::setStatus(link Block, enum blockStatus status)
// Set the Block's status to status
{
	if (Block != NULL)
	{
		Block->size &= ~1U;
		Block->size |= status;
	}
}
/*..............................................................*/
void* list::getNextInMem(link Block)
// Return Block's next block in memory or NULL if no next block
{
	link nextBlock;
	if (Block != NULL && heapEnd != NULL && Block < (link)heapEnd)
	{
		nextBlock = Block + getBlockSize(Block); // get a pointer to the start of the next block
		if (nextBlock >= (link)(char*)heapEnd) // check if the next block is out of the reserved heap
			return NULL;

		return nextBlock;
	}
	else
		return NULL;
}
// /*..............................................................*/
void* list::getPrevInMem(link Block)
// Return Block's previous block in memory or NULL if no previous block
{
	link prevBlockFooter;
	link prevBlock;

	if (Block != NULL && heapStart != NULL && Block > (link)heapStart)
	{
		prevBlockFooter = (link)((char*)Block - getUnitSize()); // get a pointer to the previous block's footer to get the size of the previous block
		long long prevBlockSize = prevBlockFooter->size;
		prevBlock = Block - prevBlockSize; // get a pointer to the start of the previous block
		if (getBlockSize(prevBlock) <= 2 || prevBlock < (link)(char*)heapStart) // check if the block size <2 or the previous block is out of the reserved heap
			return NULL;
		return prevBlock;
	}
	else
		return NULL;
}
/*..............................................................*/
void* list::memalloc(unsigned long long reqSize)
// Returns a pointer to user which can use
{
	allocated = false;
	int unitSize = getUnitSize(); // get the size of struct
	long long size = convertBytesToUnits(reqSize);
	long long totalSize = (size > 0) ? (size + 2) : 0; // adding 2 units: 1 for header and 1 for  footer
	extSize = totalSize * getUnitSize();			// variable related to external Fragmentation

	static link chunkStart; // a pointer to the block start
	chunkSize = (freeChunkStart >= heapEnd) ? 0 : convertBytesToUnits(heapEnd - freeChunkStart);

	if (head == NULL) // Case 1: Heap isn't full and there aren't free blocks (case 2)
	{
		if (!(totalSize <= chunkSize) || freeChunkStart >= heapEnd)
		{
			allocFailsCounter++;
			return NULL;
		}
		chunkStart = (link)freeChunkStart;
		freeChunkStart = (char*)(chunkStart + totalSize); // move the free chunk pointer to the new start of  the big free chunk
		setUnits((link)chunkStart, totalSize);
		setStatus((link)chunkStart, INUSE);
		allocated = true;
		mp.insert(std::pair<void*, int>((void*)((char*)chunkStart + unitSize), (size * getUnitSize() - reqSize)));
		if (extFrag == true)
			extFragmentation -= extSize;
		heapRemaining -= totalSize * getUnitSize();
		return (void*)((char*)chunkStart + unitSize);
	}

	else if (head != NULL) // Heap isn't full and there are free blocks
	{
		link firstFitBlock = NULL;
		for (chunkStart = head; chunkStart != NULL; chunkStart = (link)getNext(chunkStart)) // loop on free blocks
		{
			if (totalSize == getBlockSize(chunkStart)) // check if requested size = free block size
			{
				removeFromList(chunkStart); // remove the free block from the free linked list
				setUnits(chunkStart, totalSize);
				setStatus(chunkStart, INUSE);
				allocated = true;
				mp.insert(std::pair<void*, int>((void*)((char*)chunkStart + unitSize), (size * getUnitSize() - reqSize)));
				if (extFrag == true)
				{
					for (int j = 0; j < count; j++)
					{
						if (freeForExt[j] == (void*)((char*)chunkStart))
							extFragmentation -= extSize;
					}
				}
				heapRemaining -= totalSize * getUnitSize();
				return (void*)((char*)chunkStart + unitSize);
			}
			if (totalSize < getBlockSize(chunkStart)) // check if requested size = free block size
			{
				firstFitBlock = chunkStart;
				break;
			}
		}

		if (totalSize <= chunkSize) // allocate a block from the big free chunk if the requested size <= available size
		{
			if (freeChunkStart >= heapEnd)
			{
				allocated = false;
				allocFailsCounter++;
				return NULL;
			}
			chunkStart = (link)freeChunkStart;
			freeChunkStart = (char*)(chunkStart + totalSize); // move the free chunk pointer to the new start of  the big free chunk
			setUnits(chunkStart, totalSize);
			setStatus(chunkStart, INUSE);
			allocated = true;
			mp.insert(std::pair<void*, long long>((void*)((char*)chunkStart + unitSize), (size * getUnitSize() - reqSize)));
			if (extFrag == true)
				extFragmentation -= extSize;
			heapRemaining -= totalSize * getUnitSize();
			return (void*)((char*)chunkStart + unitSize);
		}
		if (totalSize < getBlockSize(firstFitBlock)) // check if requested size < free block size
		{
			if (totalSize >= getBlockSize(firstFitBlock) - 2) // if the free block size is a slightly larger than the requested size
			{
				removeFromList(firstFitBlock); // remove the free block from the free linked list
				setStatus(firstFitBlock, INUSE);
				allocated = true;
				mp.insert(std::pair<void*, long long>((void*)((char*)firstFitBlock + unitSize), ( (getBlockSize(firstFitBlock)* getUnitSize()) - reqSize - 32)));
				if (extFrag == true)
				{
					for (int j = 0; j < count; j++)
					{
						if (freeForExt[j] == (void*)((char*)chunkStart))
							extFragmentation -= extSize;
					}
				}
				heapRemaining -= getBlockSize(firstFitBlock) * getUnitSize();
				return (void*)((char*)firstFitBlock + unitSize);
			}
			else // if the free block size is large compared to the requested size
			{
				splitChunk(firstFitBlock, totalSize); // split the free block into 2 block
				allocated = true;
				mp.insert(std::pair<void*, long long>((void*)((char*)firstFitBlock + unitSize), (size * getUnitSize() - reqSize)));
				if (extFrag == true)
				{
					for (int j = 0; j < count; j++)
					{
						if (freeForExt[j] == (void*)((char*)chunkStart))
							extFragmentation -= extSize;
					}
				}
				heapRemaining -= totalSize * getUnitSize();
				return (void*)((char*)firstFitBlock + unitSize);
			}
		}
	}
	allocated = false;
	external = computeExtFragmentation();
	lastExternal = extSize;
	allocFailsCounter++;
	return NULL;					// NO ALLOCATION ( full heap + NO free blocks )
}
/*..............................................................*/
void list::removeFromList(link Block)
// remove the block pointed by Block from the free linked list
{
	//printFreeList();
	if (!checkInList(Block))
	{
		allocFailsCounter++;
		//printf("removeFromList failed\n");
		return;
	}

	if (getPrev(Block) == NULL) // check if the block pointed by Block is the first block
	{
		head = (link)getNext(Block);
		setNext((link)getPrev(Block), (link)getNext(Block)); // set Block->next->prev = Block->prev , Block->prev->next = Block->next
	}
	else if (getNext(Block) == NULL) // check if the block pointed by Block is the last block
	{
		setNext((link)getPrev(Block), (link)getNext(Block)); // set Block->next->prev = Block->prev , Block->prev->next = Block->next
	}
	else // mid block
	{
		setNext((link)getPrev(Block), (link)getNext(Block)); // set Block->next->prev = Block->prev , Block->prev->next = Block->next
	}
	(Block + getBlockSize(Block) - 1)->adjChunk = NULL; // block->next = NULL
	Block->adjChunk = NULL; // block->prev = NULL
}
/*..............................................................*/
void list::splitChunk(link chunkStart, long long totalSize)
// split the free block pointed by chunkStart into 2 blocks , and use the first block and insert the second block to the free linked list
{
	long long chunkStartSize = getBlockSize(chunkStart);
	link splitPtr;
	removeFromList(chunkStart); // remove the block pointed by chunkStart from the free linked list
	setUnits(chunkStart, totalSize);
	setStatus(chunkStart, INUSE);
	splitPtr = (link)getNextInMem(chunkStart); // get a pointer to the second block which will be inserted again to the linked list
	setUnits(splitPtr, (int)chunkStartSize - totalSize);		// may crash due to type_Casting
	setStatus(splitPtr, FREE);
	insertFree(splitPtr);
}
/*..............................................................*/
void list::memfree(void* dataPtr) // dataPtr is the pointer which is returned to the user by memalloc()
{
	if (dataPtr == NULL)
	{
		freeFailsCounter++;
		return;
	}
	bool valid = false;
	std::map<void*, long long>::iterator itr;
	for (itr = mp.begin(); itr != mp.end(); ++itr)
	{
		if (itr->first == dataPtr)
		{
			valid = true;
			mp.erase(dataPtr);
			break;
		}
	}
	if (!valid)
	{
		freeFailsCounter++;
		printf("Not allocated!\n");
		return;
	}

	link freeBlockStart, nextBlockInMem, prevBlockInMem;
	freeBlockStart = (link)((char*)dataPtr - getUnitSize()); // move the pointer to the free block start
	if (!(checkValidBlock(freeBlockStart)))
		return;

	dataPtr = NULL;
	if (getStatus(freeBlockStart) == FREE) // check if it's already freed before
	{
		FailsCount++;
		return;
	}

	heapRemaining += getBlockSize(freeBlockStart) * getUnitSize();
	setStatus(freeBlockStart, FREE);
	prevBlockInMem = (link)getPrevInMem(freeBlockStart); // get the previous block in memory (adjacent block)
	nextBlockInMem = (link)getNextInMem(freeBlockStart); // get the next block in memory (adjacent block)

	if (nextBlockInMem != NULL && getStatus(nextBlockInMem) == FREE && checkInList(nextBlockInMem))
	{
		removeFromList(nextBlockInMem);
		freeBlockStart = (link)(void*)merge(freeBlockStart, nextBlockInMem); // if next block in memory is free , so coalesce them together
	}

	if (prevBlockInMem != NULL && getStatus(prevBlockInMem) == FREE && checkInList(prevBlockInMem))
	{
		removeFromList(prevBlockInMem);
		freeBlockStart = (link)(void*)merge(prevBlockInMem, freeBlockStart); // if previous block in memory is free , so coalesce them together
	}
	insertFree(freeBlockStart); // insert the free block into the inordered linked list

}
/*..............................................................*/
void list::insertFree(link Block)
// Block is a pointer to a block which will be inserted to the free linked list
{
	link ptr;
	if (Block == NULL)
	{
		freeFailsCounter++;
		return;
	}
	if (head == NULL || getBlockSize(Block) <= getBlockSize(head)) // case 1 : check if empty linked list or Block's size < head's size
	{
		if (getStatus(Block) == FREE)
		{
			setNext(Block, head);
			if (head != NULL) // check if the linked list isn't empty
			{
				setPrev(head, Block);
			}
			head = Block; // Set head = Block
			setPrev(Block, NULL);
		}
		else
			return;
	}
	else // case 2 : if Block's size > head's size
	{
		ptr = head;
		while ((link)getNext(ptr) != NULL && getBlockSize((link)getNext(ptr)) < getBlockSize(Block)) // loop till finding the right position to insert Block
		{
			if (getStatus((link)getNext(ptr)) == FREE)
				ptr = (link)getNext(ptr);
		}

		if ((link)getNext(ptr) == NULL) // if the block pointed by ptr is the last block in the free linked list
		{
			setNext(Block, NULL);
			setNext(ptr, Block);
			setPrev(Block, ptr);
		}
		else
		{
			setNext(Block, (link)getNext(ptr));
			setPrev((link)getNext(ptr), Block);
			setNext(ptr, Block);
			setPrev(Block, ptr);
		}
	}
}
/*..............................................................*/
void* list::merge(link firstBlock, link secondBlock)
// merge the two blocks which are pointed by firstBlock , secondBlock pointers and return a pointer to the created block
{
	long long units;
	units = getBlockSize(firstBlock) + getBlockSize(secondBlock); // adding sizes of the 2 blocks to make larger block
	setUnits(firstBlock, units);		// may crash due to type_Casting
	return (void*)(link)firstBlock;
}
/*..............................................................*/
bool list::checkFreeList(link Block)
{
	if ((Block->adjChunk != NULL) || ((Block + getBlockSize(Block) - 1)->adjChunk != NULL) || Block == head) // check if the block is in the free linked list
	{
		if (getFooterUnits(Block) == getBlockSize(Block) && getBlockSize(Block) > 2) // check the header and footer size , they must be equal and > 2
			return true;
	}
	return false;
}
/*..............................................................*/
bool list::checkInList(link Block)
{
	for (link pred = head; pred != NULL; pred = (pred + getBlockSize(pred) - 1)->adjChunk)
	{
		if (pred == Block)
			return true;
	}
	return false;
}
/*..............................................................*/
bool list::checkValidBlock(link Block)
{
	if ((getBlockSize(Block) == getFooterUnits(Block)) && getBlockSize(Block) > 2)
		return true;
	return false;

}
/*..............................................................*/
double list::fragmentationPercentage()
{
	/*
		Sigma(free) - freeMax
		---------------------- * 100
			Sigma(free)
	*/
	if (head == NULL)
		return 0;
	link ptr, lastBlock = NULL;
	double freeBlocks(0), freeMax(0);
	for (ptr = head; ptr != NULL; ptr = (link)getNext(ptr))
	{
		freeBlocks += getBlockSize(ptr);
		lastBlock = ptr;
	}
	if (lastBlock->adjChunk == NULL)
		return 0;
	freeMax = (double)getBlockSize(lastBlock);
	return (freeBlocks - freeMax) / freeBlocks * 100;
}
/*..............................................................*/
long long list::internalFragmentation()
{
	long long intFragmentation(0);
	std::map<void*, long long>::iterator itr;
	for (itr = mp.begin(); itr != mp.end(); ++itr)
		intFragmentation += itr->second;

	return intFragmentation;
}
/*..............................................................*/
long long list::computeExtFragmentation()
{
	link ptr;
	count = 0;
	extFragmentation = chunkSize;

	for (ptr = head; ptr != NULL; ptr = (ptr + getBlockSize(ptr) - 1)->adjChunk)
	{
		freeForExt[count] = ptr;
		extFragmentation += getBlockSize(ptr) * 16;
		count++;
	}
	if (extFragmentation >= lastExternal)
	{
		extFrag = true;
		return extFragmentation;

	}
	return 0;
}
/*..............................................................*/
void list::show(void* p)
{
	link ptr;
	ptr = (link)((char*)p - getUnitSize());
	std::cout << "    Block Size = " << getBlockSize(ptr) << " units" << std::endl;
}
/*..............................................................*/
void list::printHeapInfo()
{
	std::cout << "Heap Start = " << (void*)heapStart << '\n';
	std::cout << "Heap End = " << (void*)heapEnd << '\n';
	std::cout << "Total Heap Size = " << (heapEnd - heapStart) << " Bytes" << '\n';
}
/*..............................................................*/
void list::printFreeList()
{
	link pred;
	std::cout << "Free Linked List : " << std::endl;
	int i = 1;
	for (pred = head; pred != NULL; pred = (pred + getBlockSize(pred) - 1)->adjChunk)
	{
		std::cout << "Free Block " << i << " = " << pred;
		std::cout << "    Block Size = " << getBlockSize(pred) << " Units" << std::endl;
		i++;
	}
}