#pragma once

#ifdef LINUX
typedef int HWAVEOUT;
typedef int WAVEFORMATEX;
typedef int HWAVEOUT;

struct WAVEHDR
{
	char *lpData;
	unsigned int dwBufferLength;
};

int waveOutPrepareHeader(HWAVEOUT hwo, WAVEHDR *pwh,  UINT cbwh);
int waveOutUnprepareHeader(HWAVEOUT hwo, WAVEHDR *pwh,  UINT cbwh);
#endif

class SoundBuffer
{
public:
	HWAVEOUT SC;
	WAVEHDR WaveHeader;
	__int16 *WaveData;
	int done;
	int sent;
	int samples;

	int del;

	SoundBuffer(int samps, HWAVEOUT sc);

	SoundBuffer(SoundBuffer &old)
	{
		WaveData=old.WaveData;
		old.WaveData=NULL;
		WaveHeader=old.WaveHeader;
		WaveHeader.lpData = (char*)WaveData;
		old.WaveHeader.lpData = NULL;
		SC=old.SC;
		done=old.done;
		sent=old.sent;
		samples=old.samples;
		del=old.del;
	}

	virtual ~SoundBuffer();
};

#ifdef WIN32
void CALLBACK RaywaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
#endif

class RaySound
{
public:
	int OutBuffers;
	LinkedList2<SoundBuffer,char*,RayHeapList> buffers;
	CriticalSection cs;

	HWAVEOUT SC;
	WAVEFORMATEX WaveFormat;

	void QueueSB(SoundBuffer &sb);

	RaySound();
	RaySound(unsigned int SampleRate);
	~RaySound();

	int Proc();
};

