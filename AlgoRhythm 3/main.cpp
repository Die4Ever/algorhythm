//g++ -o algorhythm.exe main.cpp MusicPlayer.cpp rayrand.cpp raysound.cpp Song.cpp
// /opt/icecast/bin/icecast -c /opt/icecast/etc/icecast.xml
// ./algorhythm.exe | lame -r - - | /opt/icecast/bin/ezstream -c /opt/icecast/etc/ezstream_mp3.xml

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <MMSYSTEM.h>
#endif

#include <math.h>
#include <time.h>
#include <vector>
#include <string>
using namespace std;
#define RAYHEADERSMAIN
#include <util.h>
#include <oswrapper.h>
#include <RayRand.h>
//#include "asciigraph.h"
//#include <raystringstuff.h>
#include <rayheap.h>
#include <rayheapnew.h>
#include <raycontainers2.h>
#include <interpolation.h>
#include <rayhashtable.h>
#include <raystringstuff.h>
#include "raysine.h"

#include "raysound.h"
#include "MusicPlayer.h"
#include "Song.h"

int samplerate = 44100;

//const unsigned int numphases = 1;
//double phases[numphases] = {0.0};

//RaySound rs(samplerate);

int main()
{
	if( sizeof(void*) != sizeof(UINT_PTR) )
	{
		cerr << "sizeof(void*) == "<<sizeof(void*)<<"\n";
		cerr << "sizeof(UINT_PTR) == "<<sizeof(UINT_PTR)<<"\n";
		return 0;
	}
	/*double tone = tonenumerator/ 440.0;
	double step = 1.0/tone;
	for(unsigned int p=0;p<numphases;p++)
		phases[p] = double(p)*0.2;*/

	while(1)
	{
		MusicSynth ms;
		Song *song = NewSong(&ms);
		song->Play();
		delete song;
		for(int i=0;i<2;i++)
		{
			char buffer[44100*4];
			memset(buffer, 0, 44100*4);
			fwrite(buffer, 1, 44100*4, stdout);
		}
	}

	/*while(1)
	//for(int f=0;f<20;f++)
	{
		while(rs.Proc()>3)
			Sleep(1);

		SoundBuffer sb(65536, rs.SC);
		for(int i=0;i<65536; i++)
		{
			double d = 0.0;
			for(unsigned int p=0;p<numphases;p++)
				d+=rsin(phases[p]+=step);

			sb.WaveData[i] = (__int16)(d*10000.0);
		}
		rs.QueueSB( sb );
	}*/

	return 0;
}
