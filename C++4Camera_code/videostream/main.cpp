
#include "LIVE555.h"

char eventLoopWatchVariable[4];

Mat* frame;
TaskScheduler* scheduler[4];
UsageEnvironment* env[4];

unsigned WINAPI FrameMove_CAM(void *arg)
{
	int* mid = (int*)arg;
	int id = *mid;
	free(arg);
	scheduler[id] = BasicTaskScheduler::createNew();
	env[id] = BasicUsageEnvironment::createNew(*scheduler[id]);
	while(1)
	{
		ConnectCam[id]=false;
		char urlname[255];
		sprintf(urlname,"URL%d.txt",id+1);
		char url[255];
		FILE* fp = fopen(urlname,"r");
		fgets(url,255,fp);
		fclose(fp);
		openURL(*env[id],"", url,id,&frame[id]);
		
		eventLoopWatchVariable[id]=0;
		env[id]->taskScheduler().doEventLoop(&eventLoopWatchVariable[id]);

		shutdownStream(grtspClient[id]);
		printf("finish\n");
		Sleep(1000.0f);
	}
	env[id]->reclaim();
	delete scheduler[id];
	scheduler[id]= NULL;
	env[id]= NULL;
	return 0;
}

struct Pair {

	unsigned char value[640*480*3];
};

struct Pair1 {

	unsigned char value[5];
};

struct Pair2 {

	int value[1];
};


void Release()
{
	for(int i=0;i<4;i++)
	{
		env[i]->reclaim();
		delete scheduler[i];
	}
	for(int i=0; i<4;i++)
		g_Refresh[i]=true;

}


int main() 
{
	for(int i=0; i<4;i++)
	{
		eventLoopWatchVariable[i]=0;
		g_Refresh[i]=true;
		ReCon[i]=false;
		ConnectCam[i]==false;
	}
	frame= new Mat[4];
	int	BState[4];
	bool	 Refresh=false;

	bool	BStop[4];
	DWORD	BeforRefreshtime1;
	DWORD	BeforRefreshtime;
	DWORD	BeforTime[4];
	DWORD	BeforFirstConnect[4];
	for(int i=0; i<4;i++)
	{
		BeforTime[i] =GetTickCount();
		BStop[i]=true;
		BState[i]=0;
		g_State_Cam[i]=0;
		frame[i].release();
	}

	HANDLE hThread[4];
	for(int i=0; i<4;i++)
	{
		int *id = (int*)malloc(sizeof(int));
		*id= i;
		hThread[i]=(HANDLE)_beginthreadex(NULL, 0, FrameMove_CAM, id, 0, (unsigned *)0);
	}
	atexit(Release);

	Mat img(480*2,640*2,CV_8UC3); 

	HANDLE handle = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Pair), L"MySharedMemory");
	struct Pair* p = (struct Pair*) MapViewOfFile(handle, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, sizeof(Pair));

	

	HANDLE handle1 = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Pair1), L"MySharedMemory1");
	struct Pair1* p1 = (struct Pair1*) MapViewOfFile(handle1, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, sizeof(Pair1));


	HANDLE handle2 = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Pair2), L"MySharedMemory3");
	struct Pair2* refreshTime = (struct Pair2*) MapViewOfFile(handle2, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, sizeof(Pair2));


	refreshTime->value[0]= 10;

	p1->value[0] = 0;
	p1->value[1] = 0;
	p1->value[2] = 0;
	p1->value[3] = 0;
	p1->value[4] = 0;			

	MSG Message;
	ZeroMemory( &Message, sizeof( Message ) );

	BeforRefreshtime = GetTickCount();
	BeforRefreshtime1 = GetTickCount();
	for(int i=0; i<4;i++)
	{
		BeforFirstConnect[i] =GetTickCount();
	}



	while( Message.message != WM_QUIT)
	{
		if( PeekMessage( &Message, NULL, 0U, 0U, PM_REMOVE ) )
		{

			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		else
		{
			for(int i=0; i<4;i++)
			{
				if(ConnectCam[i]==false)
				{
					if((GetTickCount()-BeforFirstConnect[i] )>=4000.0f)
					{
						
						eventLoopWatchVariable[i]=1;
						
						BeforFirstConnect[i] =GetTickCount();
					}
				}
				
			}
			/*printf("%d\n",refreshTime->value[0]);*/
			if((GetTickCount()-BeforRefreshtime)>=(refreshTime->value[0]*60*1000.0f)) // REfresh
			{
				for(int i=0; i<4;i++)
					g_Refresh[i]=true;

			
				Refresh=true;
				BeforRefreshtime= GetTickCount();
			}

			if(p1->value[4]!=0) // REfresh by button
			{
				printf("REFRESH!!\n");
				for(int i=0; i<4;i++)
					g_Refresh[i]=true;


				Refresh=true;
				BeforRefreshtime= GetTickCount();
				p1->value[4] =0;
			}

			if(Refresh==true)
			{
				if((GetTickCount()-BeforRefreshtime1)>=2000.0f)
				{
					Refresh=false;
					BeforRefreshtime1= GetTickCount();
				}
			}
			else
			{
				BeforRefreshtime1= GetTickCount();
			}
			for(int i=0; i<4;i++)
			{

				if(g_State_Cam[i]>=10)
				{
					if(BState[i]==g_State_Cam[i])
					{

						BStop[i]=true;
		
					}
					else
					{
						BeforTime[i] =GetTickCount();
						BStop[i] =false;
						p1->value[i] = 1;
						/*p1->value[1] = 1;
						p1->value[2] = 1;
						p1->value[3] = 1;*/
					}

					if(BStop[i]==true)
					{
						if((GetTickCount()-BeforTime[i])>=2000.0f)
						{
							printf("Cam%d is Stop\n",i);
							if(ReCon[i]==false)
							{
								eventLoopWatchVariable[i]=1;
								ReCon[i]=true;
							}
							p1->value[i] = 0;

						}
					}
				}
				if(frame[i].empty()!=true)
				{
					int dx = i%2;
					int dy = i/2;
					frame[i].copyTo(img(Rect((dx*640),(dy*480),640,480)));
				}
				BState[i] = g_State_Cam[i];
			}
			if(Refresh==false)
			{
				Mat dst;
				resize(img, dst, Size(640,480), 0, 0, CV_INTER_LINEAR);

				for(int x=0; x<640;x++)
				{
					for(int y=0; y<480;y++)
					{
						int id = (x*480*3)+(y*3);
						p->value[id] = dst.at<Vec3b>(y,x)[0];
						p->value[id+1] = dst.at<Vec3b>(y,x)[1];
						p->value[id+2] = dst.at<Vec3b>(y,x)[2];
					}
				}


			imshow("C++IMAGE",dst);
				dst.release();
			}
			waitKey(1);
		}

	}

	if (handle != NULL)
		CloseHandle(handle);

	//img.release();

	//for(int i=0; i<4;i++)
	//{
	//	frame[i].release();
	//}


	for(int x=0; x<640;x++)
	{
		for(int y=0; y<480;y++)
		{
			int id = (x*480*3)+(y*3);
			p->value[id] = 0;
			p->value[id+1] = 0;
			p->value[id+2] = 0;
		}
	}

	return 0;


}

