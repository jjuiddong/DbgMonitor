// TestClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../Common/Common/common.h"
using namespace common;
using namespace std;

int main()
{
	cShmmem shmem;
	shmem.Init("MiningShmem");

	struct sSharedData
	{
		int state; // 0:readable,writable , 1:write, 2:write-end, 3:read
		double dtVal;
		char dummy[256];
	};
	sSharedData *sharedData = (sSharedData*)shmem.m_memPtr;

	float phase = 0;
	while (1)
	{
		while ((1 == sharedData->state) || (3 == sharedData->state))
			Sleep(1);

		sharedData->state = 1;
		sharedData->dtVal = cos(phase);
		sharedData->state = 2;

		phase += 0.1f;
		cout << "write " << phase << endl;
		Sleep(10);
	}

    return 0;
}

