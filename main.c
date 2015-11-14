#include <stdio.h>
//#include <std.h>
#include <string.h>
//#include <windows.h>

#define DEBUG_BUF


// This program is a demonstration. moreover its a experiment to 
// unsertand the usage of producer and consumer mechanics  of ring buffer.
// This will also help to understand how to use the ring buffer in real senario.

// Producer: Will fill the ring buffer a) randomly, b) At a specified rate.

// Comsumer: Will consume the data from the ring buffer and place it in a temporary 
//    		 buffer. It will read a) randomly, b) At a specified rate. 

// Ring buffer Library: It wil have the mechanisam to add the byte into the ring
//						buffer and take care of the buffer wrap around and other
// 						related parameters.


// ***** ring buffer handler functions. *****

// Ring buffer struct
// {
// base pointer
// buffer size
// tail pointer
// head pointer
// }
//
//
typedef struct
{
	char *basePtr;
	char *tailPtr;
	char *headPtr;
	int  sizeOfBuffer;	
} tRingBuf;


// Initializes the memory for the specified size of buffer
// Reset the parameters
int RingBuf_init(tRingBuf *buf, int size)
{
	int error = 0;
	buf->basePtr = NULL;
	buf->tailPtr = NULL;
	buf->headPtr = NULL;
	
	buf->basePtr = (char*) calloc(size, sizeof(char));
	
	if (buf->basePtr != NULL)
	{
		buf->tailPtr = buf->basePtr;
		buf->headPtr = buf->basePtr;
		buf->sizeOfBuffer = size;
	}
	else 
	{
		error = 1;
	}
	return error;
}

// Deinitializes the memory and reset the ring buffer.
int RingBuf_deinit(tRingBuf *buf)
{	
	if (buf != NULL)
	{
		free(buf->basePtr);
		RingBuf_reset(buf);
		return 0;
	}
	return 1;
}

// Reset the ring buffer parameters to default values.
int RingBuf_reset(tRingBuf *buf)
{
	buf->tailPtr = buf->basePtr;
	buf->headPtr = buf->basePtr;
	return 0;
}

// Read data from the ring buffer. Check for wrp condition
int RingBuf_pop(tRingBuf *buf, void* data, int size)
{
	int size1 = 0, size2 = 0;
	void *ptr = data;
	
	
	if (((buf->headPtr - buf->tailPtr) < 0) && 
	    ((buf->tailPtr + size) > (buf->basePtr + buf->sizeOfBuffer))
	   )
	{	// wrap  condition
		size1 = (buf->basePtr + buf->sizeOfBuffer) - (buf->tailPtr);
		size2 = size - size1;
		if (size2 > (buf->headPtr - buf->basePtr))
		{
			size2 = buf->headPtr - buf->basePtr;
		}
	}
	else if ((buf->tailPtr <= buf->headPtr)&&(buf->tailPtr + size) > buf->headPtr)
	{
		// less data in buffer
		size1 = buf->headPtr - buf->tailPtr;
		size2 = 0;
	}
	else
	{
		// normal case
		size1 = size; 
		size2 = 0;
	}
	
	memcpy(ptr, buf->tailPtr, size1);
	#ifdef DEBUG_BUF
	{
		int i = 0;
		printf("pop1:\t");
		while(i < size1)
		{
			printf("%d ", *(((char*)ptr)+i));
			i++;
		}
		printf("\n");		
	}
	#endif
	ptr += size1;
	buf->tailPtr += size1;
	if (size2 > 0)
	{	// due to wrap
		buf->tailPtr = buf->basePtr;
		memcpy(ptr, buf->tailPtr, size2);
		#ifdef DEBUG_BUF
		{
			int i = 0;
			printf("pop2:\t");
			while(i < size2)
			{
				printf("%d ", *(((char*)ptr)+i));
				i++;
			}
			printf("\n");		
		}
		#endif
		buf->tailPtr += size2;
	}
	
	return (size1 + size2);
}

// Initializes the memory for the specified size buffer
int RingBuf_push(tRingBuf *buf, void* data, int size)
{
	int size1 = 0, size2 = 0;
	void *ptr = data;
	
	if ((buf->headPtr + size) > (buf->basePtr + buf->sizeOfBuffer))
	{
		// wrap condition
		size1 = (buf->basePtr + buf->sizeOfBuffer) - buf->headPtr;
		size2 = size - size1;
	}
	else
	{
		size1 = size;
		size2 = 0;
	}
	
	// tail consideration .. move the tail if head reaches the tail
	if ((((buf->headPtr + size1) > buf->tailPtr) && (buf->headPtr < buf->tailPtr)) ||
		 (buf->tailPtr < (buf->basePtr + size2))
	   )
	{
		if (size2 > 0)
		{
			buf->tailPtr = buf->basePtr + size2;
		}
		else
		{
			buf->tailPtr = buf->headPtr + size1;
		}	
	}
	
	memcpy(buf->headPtr, ptr, size1);
	#ifdef DEBUG_BUF
	{
		int i = 0;
		printf("pushing1:\t");
		while(i < size1)
		{
			printf("%d ", *((char*)ptr+i));
			i++;
		}
		printf("\n");		
	}
	#endif
	ptr += size1;
	buf->headPtr += size1;
	if (size2 > 0)
	{
		buf->headPtr = buf->basePtr;
		memcpy(buf->headPtr, ptr, size2);
		#ifdef DEBUG_BUF
		{
			int i = 0;
			printf("pushing2:\t");
			while(i < size2)
			{
				printf("%d ", *(((char*)ptr)+i));
				i++;
			}
			printf("\n");		
		}
		#endif
		buf->headPtr += size2;
	}
	return (size1+size2);
}

// handling of the multiple access request from different 
// threads. Waits until the lock is not released.
int RingBuf_claim(tRingBuf *buf, int size)
{
	
}

#ifdef DEBUG_BUF
int RingBuf_dump(tRingBuf *buf)
{
	int i = 0;
	printf("Ring Buf:\t");
	while (i < buf->sizeOfBuffer)
	{
		printf("%d ", buf->basePtr[i]);
		i++;
	}
	printf("\n");
	return 0;
}
#endif





int main()
{
	int i;
	tRingBuf ringBuf;
	
	// test for the ring buffer functions
	char testData[25] = {1, 2,3, 4, 5, 6, 7, 8, 9, 10,
						 11,12,13,14,15,16,17,18,19,20,
						 21,22,23,24,25};
	char readData[20] = {0};
	  
	RingBuf_init(&ringBuf, 20);
	#ifdef DEBUG_BUF
	RingBuf_dump(&ringBuf);
	#endif
	i = RingBuf_pop(&ringBuf, readData, 13);
	#ifdef DEBUG_BUF
	RingBuf_dump(&ringBuf);
	#endif
	i = RingBuf_push(&ringBuf, testData, 2);
	#ifdef DEBUG_BUF
	RingBuf_dump(&ringBuf);
	#endif

	i = RingBuf_push(&ringBuf, &testData[2], 2);
	#ifdef DEBUG_BUF
	RingBuf_dump(&ringBuf);
	#endif
	i = RingBuf_pop(&ringBuf, readData, 1);
	#ifdef DEBUG_BUF
	RingBuf_dump(&ringBuf);
	#endif
	i = RingBuf_pop(&ringBuf, readData, 2);
	#ifdef DEBUG_BUF
	RingBuf_dump(&ringBuf);
	#endif
	i = RingBuf_push(&ringBuf, &testData[1], 24);
	#ifdef DEBUG_BUF
	RingBuf_dump(&ringBuf);
	#endif
	i = RingBuf_pop(&ringBuf, readData, 12);
	#ifdef DEBUG_BUF
	RingBuf_dump(&ringBuf);
	#endif
	i = RingBuf_push(&ringBuf, &testData[0], 10);
	#ifdef DEBUG_BUF
	RingBuf_dump(&ringBuf);
	#endif
	i = RingBuf_push(&ringBuf, &testData[0], 10);
	#ifdef DEBUG_BUF
	RingBuf_dump(&ringBuf);
	#endif
	i = RingBuf_pop(&ringBuf, readData, 13);
	#ifdef DEBUG_BUF
	RingBuf_dump(&ringBuf);
	#endif

	RingBuf_deinit(&ringBuf);

	// initiate a consumer thread.
	
	
	// initiate a producer thread. 
	
}





