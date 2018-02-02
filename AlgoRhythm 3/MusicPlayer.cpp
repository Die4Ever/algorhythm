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
#include "raysine.h"

#include "raysound.h"

#include "MusicPlayer.h"

const int BLOCK_SIZE = 8192;

double tonenumerator = 44100.0 / 6.283185307179586476925286766559;
RaySine<65536> rsin;

double NoteFreqs[128] = {0.0};

class Note
{
public:
	double freq;
	double phase;
	int len;
	unsigned __int64 start;
	double vel;
	double pan;
	double mod;

	int SortComp(Note *objB)
	{
		if( start < objB->start )
			return -1;
		else if( start > start )
			return 1;
		return 0;
	}

	int SortComp(unsigned __int64 objB)
	{
		if( start < objB )
			return -1;
		else if( start > objB )
			return 1;
		return 0;
	}

};

class Instrument
{
public:
	AVLTree2<Note, unsigned __int64, RayHeapList> *Notes;
	int ID;
	int attack;
	int release;

	Instrument(int id, unsigned __int64 tags)
	{
		attack=2000;
		release=4000;
		if(tags&1)
		{
			attack=4000;
			release=20000;
		}
		ID=id;
		Notes = new AVLTree2<Note, unsigned __int64, RayHeapList>(8192);
	}

	Instrument(Instrument &old)
	{
		//cout << "Copy con\n";
		ID=old.ID;
		Notes=old.Notes;
		old.Notes=NULL;
	}
	
#ifdef WIN32
	Instrument(Instrument &&old)
	{
		ID=old.ID;
		Notes=old.Notes;
		old.Notes=NULL;
	}
#endif

	virtual ~Instrument()
	{
		delete Notes;
	}

	virtual void Play(unsigned __int64 time, double *lsamples, double *rsamples)
	{
		Note *n2=NULL;
		for(Note *n=Notes->GetFirst(); n; n=n2)
		{
			if(n->start+n->len < time)
			{
				n2 = Notes->GetNext(n);
				Notes->Delete(n);
				continue;
			}
			if(n->start >= time + BLOCK_SIZE)
				return;
			n2 = Notes->GetNext(n);

			int s = int(((__int64)n->start)-((__int64)time));
			int e = int(s + n->len);
			int e2 = min(BLOCK_SIZE-1, e);
			int s2 = max(0, s);

			double tone = tonenumerator/ n->freq;
			double step = 1.0/tone;

			for(int i=s2;i<=e2;i++)
			//for(int i=0;i<16384;i++)
			{
				double vel = n->vel;
				int pos = i-s;
				int pos2 = e-i;
				if(pos<attack)
					vel*=double(pos)/double(attack);
				if(pos2<release)
					vel*=double(pos2)/double(release);

				//lsamples[i] += vel*((rsin(n->phase*0.9995))>0.0 ? 0.25 : -0.25);
				//rsamples[i] += vel*((rsin(n->phase*1.0005))>0.0 ? 0.25 : -0.25);
				lsamples[i] += vel*((rsin(n->phase))>0.0 ? 0.25 : -0.25) * n->pan;
				rsamples[i] += vel*((rsin(n->phase))>0.0 ? 0.25 : -0.25) * (1.0-n->pan);
				n->phase += step;
			}
		}
	}

	virtual void AddNote(double start_time, double length, unsigned char note, double velocity, double pan, double mod)
	{
		Note cnote;
		cnote.freq  = NoteFreqs[note];
		cnote.phase = 0.0;
		cnote.vel = velocity;
		cnote.pan = pan;
		cnote.mod = mod;
		cnote.len = int(length/1000.0 * 44100.0);
		cnote.start = (unsigned __int64)(start_time*44100.0/1000.0);
		Notes->Create( cnote );
	}
};

class SineSynth : public Instrument
{
public:

	SineSynth(int id, unsigned __int64 tags) : Instrument(id, tags)
	{
	}

	virtual void Play(unsigned __int64 time, double *lsamples, double *rsamples)
	{
		Note *n2=NULL;
		for(Note *n=Notes->GetFirst(); n; n=n2)
		{
			if(n->start+n->len < time)
			{
				n2 = Notes->GetNext(n);
				Notes->Delete(n);
				continue;
			}
			if(n->start >= time + BLOCK_SIZE)
				return;
			n2 = Notes->GetNext(n);

			int s = int(((__int64)n->start)-((__int64)time));
			int e = int(s + n->len);
			int e2 = min(BLOCK_SIZE-1, e);
			int s2 = max(0, s);

			double tone = tonenumerator/ n->freq;
			double step = 1.0/tone;

			for(int i=s2;i<=e2;i++)
			//for(int i=0;i<16384;i++)
			{
				double vel = n->vel;
				int pos = i-s;
				int pos2 = e-i;
				if(pos<attack)
					vel*=double(pos)/double(attack);
				if(pos2<release)
					vel*=double(pos2)/double(release);

				vel *= 0.75;

				lsamples[i] += (vel*rsin(n->phase* (1.0+0.0002) )) * n->pan;
				rsamples[i] += (vel*rsin(n->phase* (1.0-0.0002) )) * (1.0-n->pan);

				lsamples[i] += (vel*rsin(n->phase* (1.0-0.00021) )*0.25) * n->pan;
				rsamples[i] += (vel*rsin(n->phase* (1.0+0.00021) )*0.25) * (1.0-n->pan);

				n->phase += step;
			}
		}
	}

};

class AcidSynth : public Instrument
{
public:
	double res;

	AcidSynth(int id, unsigned __int64 tags) : Instrument(id, tags)
	{
		res=0.0;
	}

	virtual void Play(unsigned __int64 time, double *lsamples, double *rsamples)
	{
		Note *n2=NULL;
		for(Note *n=Notes->GetFirst(); n; n=n2)
		{
			if(n->start+n->len < time)
			{
				n2 = Notes->GetNext(n);
				Notes->Delete(n);
				continue;
			}
			if(n->start >= time + BLOCK_SIZE)
				return;
			n2 = Notes->GetNext(n);

			int s = int(((__int64)n->start)-((__int64)time));
			int e = int(s + n->len);
			int e2 = min(BLOCK_SIZE-1, e);
			int s2 = max(0, s);

			double tone = tonenumerator/ n->freq;
			double step = 1.0/tone;

			for(int i=s2;i<=e2;i++)
			//for(int i=0;i<16384;i++)
			{
				double vel = n->vel;
				int pos = i-s;
				int pos2 = e-i;
				if(pos<attack)
					vel*=double(pos)/double(attack);
				if(pos2<release)
					vel*=double(pos2)/double(release);

				//double sample = vel*((rsin(n->phase))>0.0 ? 0.25 : -0.25);
				double d1 = n->phase*(65536.0/6.283185307179586476925286766559)+0.5;
				unsigned int s = (unsigned int)(d1)&65535;

				double sample = double(s)/-65535.0 +0.5;
				sample *= vel*0.6;
				/*double tres=res;
				res = res*0.5+sample*0.5;

				tres*=0.5;*/
				lsamples[i] += sample * n->pan;//+tres;
				rsamples[i] += sample * (1.0-n->pan);//+tres;

				n->phase += step;
			}
		}
	}

};

class Break : public Instrument
{
public:
	double *ldata;
	double *rdata;
	int samples;
	int attack;
	int release;

	Break(int id, char *filename, double length, double volume) : Instrument(id, 0)
	{
		attack=50;
		release=50;

		samples = 0;
		ldata = NULL;
		rdata = NULL;
		ifstream file(filename, ios::binary);

		file.seekg(0, ios::end);
		samples = int(file.tellg())/4;

		if(file.bad() || samples==0)
		{
			//cerr << "Bad file - "<<filename<<"\n";
			//exit(1);
			samples=1;
			ldata=NULL;
			rdata=NULL;
			return;
		}

		double curlen = double(samples)/44100.0 *1000.0;
		if(length==0.0)
		{
			length=curlen;
			release=1000;
		}

		double speed = curlen/length;

		/*if(speed<0.7)
			speed*=2.0;
		else if(speed>3.0)
			speed*=0.25;
		else if(speed>2.0)
			speed*=0.5;*/

		short *tdata = new short[samples*2];
		file.seekg(0);
		file.read((char*)tdata, samples*4);

		cerr << "ldata == "<<(int)((double)(samples)/speed)<<", rdata == "<<(int)((double)(samples)/speed)<<"\n";
		ldata = new double[(int)((double)(samples)/speed)];
		rdata = new double[(int)((double)(samples)/speed)];

		/*for(int i=0;i<samples;i++)
		{
			ldata[int(double(i)/speed)] = double(tdata[i*2])/30000.0;
			rdata[int(double(i)/speed)] = double(tdata[i*2+1])/30000.0;
		}*/
		samples = int(double(samples/speed));
		for(int i=0;i<samples;i++)
		{
			double dist = 0.25;//double(i)*speed - double(i+1)*speed;
			short s = Interpolate<short>(tdata[int(double(i)*speed)*2], tdata[int(double(i+1)*speed)*2], dist);
			ldata[i] = double(s)/30000.0;
			ldata[i] *= volume;

			dist = 0.25;//double(i+1)*speed;
			s = Interpolate<short>(tdata[int(double(i)*speed)*2+1], tdata[int(double(i+1)*speed)*2+1], dist);
			rdata[i] = double(s)/30000.0;
			rdata[i] *= volume;
		}
		delete[] tdata;
		tdata=NULL;
		//samples = int(double(samples/speed));

		/*ldata = new double[int((double)(samples))];
		rdata = new double[int((double)(samples))];

		for(int i=0;i<samples;i++)
		{
			ldata[int(double(i))] = double(tdata[i*2])/30000.0;
			rdata[int(double(i))] = double(tdata[i*2+1])/30000.0;
		}*/
	}

#ifdef WIN32
	Break(Break &&old) : Instrument(old)
	{
		ldata=old.ldata;
		rdata=old.rdata;
		samples=old.samples;
		old.ldata=NULL;
		old.rdata=NULL;
		old.samples=0;
	}
#else
	Break(Break &old) : Instrument(old)
	{
		ldata=old.ldata;
		rdata=old.rdata;
		samples=old.samples;
		old.ldata=NULL;
		old.rdata=NULL;
		old.samples=0;
	}
#endif

	virtual ~Break()
	{
		delete[] ldata;
		ldata=NULL;
		delete[] rdata;
		rdata=NULL;
	}

	virtual void Play(unsigned __int64 time, double *lsamples, double *rsamples)
	{
		if(ldata==NULL || rdata==NULL)
			return;

		Note *n2=NULL;
		for(Note *n=Notes->GetFirst(); n; n=n2)
		{
			if(n->start+n->len < time)
			{
				n2 = Notes->GetNext(n);
				Notes->Delete(n);
				continue;
			}
			if(n->start >= time + BLOCK_SIZE)
				return;
			n2 = Notes->GetNext(n);

			int s = int(((__int64)n->start)-((__int64)time));
			int e = int(s + n->len);
			int e2 = min(BLOCK_SIZE-1, e);
			int s2 = max(0, s);

			int a = int(double(samples)*n->freq);
			if(s<0)
				a -= s;

			for(int i=s2;i<=e2 && a<samples;i++,a++)
			{
				double vel = n->vel;
				int pos = i-s;
				int pos2 = e-i;
				if(pos<attack)
					vel*=double(pos)/double(attack);
				if(pos2<release)
					vel*=double(pos2)/double(release);

				lsamples[i] += vel*ldata[a];
				rsamples[i] += vel*rdata[a];
			}
		}
	}

	virtual void AddNote(double start_time, double length, unsigned char note, double velocity, double pan, double mod)
	{
		Note cnote;
		cnote.freq  = double(note)/128.0;
		cnote.phase = 0.0;
		cnote.vel = velocity;
		cnote.len = int(length/1000.0 * 44100.0);
		cnote.start = (unsigned __int64)(start_time*44100.0/1000.0);
		cnote.pan = pan;
		cnote.mod = mod;
		Notes->Create( cnote );
	}
};

double MusicSynth::GetCounter()
{
	return double(time+BLOCK_SIZE)/44100.0*1000.0;
}

void MusicSynth::AddNote(double start_time, double length, unsigned char note, double velocity, int instrument, double pan, double mod)
{
	for(unsigned int i=0;i<instruments.size();i++)
	{
		if(instruments[i]->ID == instrument)
		{
			instruments[i]->AddNote(start_time,length,note,velocity,pan,mod);
			return;
		}
	}
}

int MusicSynth::Play()
{
	double lsamples[BLOCK_SIZE];
	double rsamples[BLOCK_SIZE];
	for(int i=0;i<BLOCK_SIZE;i++)
		rsamples[i]=lsamples[i]=0.0;

	for(unsigned int i=0;i<instruments.size();i++)
		instruments[i]->Play(time, lsamples, rsamples);

	SoundBuffer sb(BLOCK_SIZE*2, rs.SC);

	for(int i=0;i<BLOCK_SIZE;i++)
	{
		int sl = int(lsamples[i]*8192.0);
		int sr = int(rsamples[i]*8192.0);
		/*if(samples[i]>10.0 || samples[i]<-10.0)
			cout << "CLIPPING!\n";*/
		sb.WaveData[i*2] = short( sl );
		sb.WaveData[i*2+1] = short( sr );
	}

	while(rs.Proc()>1)
		RaySleep(1000);

	//outputfile.write( (char*)sb.WaveData, BLOCK_SIZE*4);
	rs.QueueSB(sb);
	time += BLOCK_SIZE;

	return rs.Proc();
}

MusicSynth::~MusicSynth()
{
	for(uint i=0;i<instruments.size();i++)
	{
		delete instruments[i];
		instruments[i]=NULL;
	}
}

MusicSynth::MusicSynth() : rs(44100)//, outputfile("output.snd", ios::binary)
{
	NoteFreqs[60] = 261.63;//C4
	NoteFreqs[61] = 277.18;
	NoteFreqs[62] = 293.66;
	NoteFreqs[63] = 311.13;
	NoteFreqs[64] = 329.63;
	NoteFreqs[65] = 349.23;
	NoteFreqs[66] = 369.99;
	NoteFreqs[67] = 392.0;
	NoteFreqs[68] = 415.3;
	NoteFreqs[69] = 440.0;//A4
	NoteFreqs[70] = 466.16;
	NoteFreqs[71] = 493.88;

	for(int i=72;i<128;i++)
		NoteFreqs[i] = NoteFreqs[i-12]*2.0;
	for(int i=59;i>=0;i--)
		NoteFreqs[i] = NoteFreqs[i+12]/2.0;

	//for(int i=0;i<128;i++)
	//	cout << i << " == "<<NoteFreqs[i]<<"\n";

	time = 0;
	InstrumentID=0;
	instruments.push_back( new Instrument(InstrumentID++,0) );
}

int MusicSynth::MakeInstrument(unsigned __int64 tags)
{
	if(tags&2)
		instruments.push_back( new AcidSynth(InstrumentID, tags) );
	else if(tags&4)
		instruments.push_back( new SineSynth(InstrumentID, tags) );
	else
		instruments.push_back( new Instrument(InstrumentID, tags) );
	return InstrumentID++;
}

int MusicSynth::MakeBreak(char *filename, double length, double volume)
{
	instruments.push_back( new Break(InstrumentID, filename, length, volume) );
	return InstrumentID++;
}
