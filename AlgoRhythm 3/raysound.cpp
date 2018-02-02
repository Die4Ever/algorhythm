#include <iostream>
#include <fstream>
#include <wchar.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <MMSYSTEM.h>
#endif

#include <vector>
using namespace std;
//#include "Fft.h"
//#include <rayrand.h>
#include <util.h>
#include <oswrapper.h>
#include <RayRand.h>

#include <raycontainers2.h>
//#include <avltree.h>
#include <rayheapnew.h>
#include <rayheap.h>
#include <interpolation.h>
//#include <raystringstuff.h>

#include "raysound.h"

#ifdef LINUX
int waveOutPrepareHeader(HWAVEOUT hwo, WAVEHDR *pwh,  UINT cbwh)
{
	return 0;
}

int waveOutUnprepareHeader(HWAVEOUT hwo, WAVEHDR *pwh,  UINT cbwh)
{
	return 0;
}

int waveOutWrite()
{
	return 0;
}

int waveOutClose(HWAVEOUT sc)
{
	return 0;
}
#endif

SoundBuffer::SoundBuffer(int samps, HWAVEOUT sc)
{
	SC = sc;
	del=0;
	samples = samps;
	done = 0;
	sent = 0;
	WaveData = new __int16[samps];
	memset(&WaveHeader, 0, sizeof(WaveHeader));
	WaveHeader.lpData = (char*)WaveData;
	WaveHeader.dwBufferLength = samps*2;
	//WaveHeader.dwFlags = WHDR_PREPARED;
	int Res = waveOutPrepareHeader( SC, &WaveHeader, sizeof(WAVEHDR));
	memset(WaveData, 0, samps*2);
}

SoundBuffer::~SoundBuffer()
{
	waveOutUnprepareHeader(SC, &WaveHeader, sizeof(WAVEHDR));
	delete[] WaveData;
}


void RaySound::QueueSB(SoundBuffer &sb)
{
#ifdef WIN32
	cs.lock();
	SoundBuffer *psb = buffers.Create<SoundBuffer>(sb);
	waveOutWrite(SC, &psb->WaveHeader, sizeof(WAVEHDR) );
	OutBuffers++;
	cs.unlock();
#else
	fwrite((char*)sb.WaveData, 2, sb.samples, stdout);
#endif
}

RaySound::RaySound()
{
	OutBuffers=0;

#ifdef WIN32
	int samplerate = 44100;
	int bytespersample = 2;
	WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormat.nChannels = 2;
	WaveFormat.cbSize = 0;
	WaveFormat.nAvgBytesPerSec = samplerate * bytespersample * WaveFormat.nChannels;
	WaveFormat.nBlockAlign = bytespersample * WaveFormat.nChannels;
	WaveFormat.nSamplesPerSec = samplerate;
	WaveFormat.wBitsPerSample = bytespersample*8;

	int e = waveOutOpen(&SC, WAVE_MAPPER, &WaveFormat, 0, 0, WAVE_FORMAT_QUERY);

	int Res = waveOutOpen(&SC, WAVE_MAPPER, &WaveFormat, (DWORD_PTR)&RaywaveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION/* | WAVE_ALLOWSYNC*/);
	e = GetLastError();
#endif
}

RaySound::RaySound(unsigned int SampleRate)
{
	OutBuffers=0;

#ifdef WIN32
	int samplerate = SampleRate;
	int bytespersample = 2;
	WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormat.nChannels = 2;
	WaveFormat.cbSize = 0;
	WaveFormat.nAvgBytesPerSec = samplerate * bytespersample * WaveFormat.nChannels;
	WaveFormat.nBlockAlign = bytespersample * WaveFormat.nChannels;
	WaveFormat.nSamplesPerSec = samplerate;
	WaveFormat.wBitsPerSample = bytespersample*8;

	int e = waveOutOpen(&SC, WAVE_MAPPER, &WaveFormat, 0, 0, WAVE_FORMAT_QUERY);

	int Res = waveOutOpen(&SC, WAVE_MAPPER, &WaveFormat, (DWORD_PTR)&RaywaveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION/* | WAVE_ALLOWSYNC*/);
	e = GetLastError();
#endif
}


RaySound::~RaySound()
{
	while(Proc())
		RaySleep(1000);
	RaySleep(1000);

	waveOutClose(SC);
}

int RaySound::Proc()
{
	cs.lock();
	for(SoundBuffer *sb=buffers.GetFirst(); sb!=NULL; sb=buffers.GetNext(sb))
	{
		if(sb->del == 1)
		{
			//delete sb;
			buffers.Delete(sb);
			break;
		}
	}
	int ob = OutBuffers;
	cs.unlock();

	return ob;
}

#ifdef WIN32
void CALLBACK RaywaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	if(uMsg == WOM_DONE)
	{
		//MakeNewOutBuffer++;
		if(dwInstance != NULL)
		{
			((RaySound*)dwInstance)->cs.lock();
			/*if(((RaySound*)dwInstance)->buffers.pHead!=NULL)
				((RaySound*)dwInstance)->buffers.pHead->del = 1;*/

			for(SoundBuffer *sb=((RaySound*)dwInstance)->buffers.GetFirst(); sb!=NULL; sb=((RaySound*)dwInstance)->buffers.GetNext(sb))
			{
				if(sb->del == 0)
				{
					sb->del = 1;
					break;
				}
			}

			((RaySound*)dwInstance)->OutBuffers--;
			((RaySound*)dwInstance)->cs.unlock();
		}
		//delete SoundOutBuffers.pHead;
	}
}
#endif
