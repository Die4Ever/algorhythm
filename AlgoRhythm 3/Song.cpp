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

Rcout rcout(stderr);

unsigned char wnotes[7*11] = { 0,2,4,5,7,9,11 };

class Note
{
public:
	double time;
	double length;
	int instrument;

	unsigned char status;
	unsigned char note;
	double velocity;
	double pan;
	double mod;

	Note()
	{
		note = 0;
		status = 144;
		pan = 0.5;
		mod = 0.0;
	}

	Note(double Time, double Length, int Instrument, unsigned char Status, unsigned char Note, double Velocity)
	{
		time = Time;
		length = Length;
		instrument = Instrument;
		status = Status;
		note = Note;
		velocity = Velocity;
		pan = 0.5;
	}

};

class Voice
{
public:
	bool lead;
	bool acid;
	bool bass;
	bool strings;
	bool pad;
	bool arp;
	bool drums;
	bool sample;
	int status;
	double power;
	double density;
	int instrument;
	bool init;

	double rhythm[1024];
	double pan;
	int key;

	Voice()
	{
		sample=drums=arp=lead=acid=bass=strings=pad=false;
		status=0;
		instrument=0;
		init=0;
		pan = 0.5;
		density = 1.0;
		key = 0;
		for(int i=0;i<1024;i++)
			rhythm[i]=0.0;
	}

	virtual ~Voice()
	{
	}

	virtual void Play(Note *notes, int phrase_length, RayRand64 &rrand, vector<Note> &newnotes)
	{
		if(init==0)
		{
			init=1;
			for(int i=0;i<16;i++)
			{
				rhythm[i] = ((double)(rrand.iRand64HQ()%1000000))/100000.0 -5.0;
				rhythm[i] *= 5.0;
			}
			for(int i=16;i<1024;i++)
			{
				//rhythm[i] = (rhythm[i%16] + ((double)(rrand.iRand64HQ()%1000000))/100000.0 -5.0)*5.0 / 2.0;
				rhythm[i] = rhythm[i%16];
			}
			for(int i=0;i<1024/16;i++)
			{
				if(rrand.iRand64HQ()%4==0)
				{
					for(int a=i*16;a<i*16+16;a++)
						rhythm[a] = 0.0;
				}
				else if(rrand.iRand64HQ()%4==0)
				{
					for(int a=i*16;a<i*16+16;a++)
						rhythm[a] -= 10.0;
				}
				else if(rrand.iRand64HQ()%4==0)
				{
					for(int a=i*16;a<i*16+16;a++)
						rhythm[a] += 5.0;
				}
			}
		}
		if(status==0)
			return;

		for(int i=0;i<phrase_length;i++)
		{
			double vel = notes[i].velocity + rhythm[i];
			if(strings)
			{
				if( ((double(i%32==0)*20 + double(i%16==0)*50.0 + vel)*density > 100.0 && i%4==0) || i==0 )
				{
					Note tnote = notes[i];
					tnote.pan = pan;
					//tnote.note = wnotes[tnote.note+7];
					tnote.note = wnotes[tnote.note+key];
					//tnote.note = notes[(i%2)+(i/16)*16].note;
//					tnote.note+=12;
					tnote.instrument = instrument;
					tnote.length = 1.0;
					tnote.velocity *= 1.3;
					newnotes.push_back(tnote);
					//tnote.note+=12;
					tnote.note = wnotes[notes[i].note+10];
					tnote.velocity *= 0.7;
					newnotes.push_back(tnote);
				}
			}
			if(arp)
			{
				Note tnote = notes[i];
				tnote.pan = 0.5;
				tnote.note = notes[(i%2)+(i/16)*16].note;
				tnote.note = wnotes[tnote.note+key];
				if(i%4>1)
					tnote.note+=12;
				tnote.note+=12;
				tnote.instrument = instrument;
				//tnote.length = 0.5;
				tnote.length = 0.05;
				//tnote.velocity *= 1.5;
				tnote.velocity = 520.0;
				newnotes.push_back(tnote);
			}
			if(lead)
			{
				if( (double(i%8==0)*50.0 + double(i%2==0)*10.0 + vel)*density > 50.0 || i==0)
				{
					Note tnote = notes[i];
					tnote.pan = pan;

					tnote.note = wnotes[tnote.note+key];
					tnote.instrument = instrument;
					tnote.length = 0.2;
					tnote.velocity += 50;
					tnote.velocity *= 1.2;
					newnotes.push_back(tnote);
				}
				if( (double(i%8==0)*50.0 + double(i%4==0)*10.0 + vel)*density > 90.0 || i==0)
				{
					Note tnote = notes[i];
					tnote.pan = pan;
					tnote.note = wnotes[tnote.note+key+2];
					tnote.instrument = instrument;
					tnote.length = 0.2;
					tnote.velocity += 50;
					//tnote.velocity *= 1.5;
					newnotes.push_back(tnote);
				}
			}
			if(bass)
			{
				//if(i%16==0)
				if( ( (double(i%32==0)*40 + double(i%16==0)*50.0 + double(i%4==0)*10.0 + vel)*density > 80.0 && i%2==0) || i==0 )
				{
					Note tnote = notes[i];
					tnote.pan = pan;
					tnote.instrument = instrument;
					tnote.note = wnotes[tnote.note+key];
					//tnote.note -= 12;
					tnote.length = 0.4;
					tnote.velocity += 120.0;
					newnotes.push_back(tnote);
				}
			}
		}
	}
};

class Drummer : public Voice
{
public:
	int motifs[4][16];
	int slots[128];

	Drummer()
	{
		drums=true;
		init=0;
		pan=0.5;
	}

	virtual void Play(Note *notes, int phrase_length, RayRand64 &rrand, vector<Note> &newnotes)
	{
		if(init==0)
		{
			for(int m=0;m<4;m++)
			{
				for(int i=0;i<16;i++)
				{
					if(rrand.iRand64HQ()%2==0 || (i%4)==0)
						motifs[m][i] = rrand.iRand64HQ()%32*4;
					else if(rrand.iRand64HQ()%2==0)
						motifs[m][i] = rrand.iRand64HQ()%64*2;
					else
						motifs[m][i] = -1;
				}
			}
			for(int i=0;i<128;i++)
				slots[i] = rrand.iRand64HQ()%4;

			init=1;
		}
		if(status==0)
			return;

		int r = rrand.iRand64HQ()%1000;
		if(r<400)
		{
			slots[rrand.iRand64HQ()%(phrase_length/8)] = rrand.iRand64HQ()%4;
			rcout << "Changing drum slot\n";
		}
		else if(r > 500 && r<800)
		{
			int s = rrand.iRand64HQ()%4;
			for(int n=0;n<16;n++)
			{
				if(rrand.iRand64HQ()%2==0 || (n%4)==0)
					motifs[s][n] = rrand.iRand64HQ()%32*4;
				else if(rrand.iRand64HQ()%2==0)
					motifs[s][n] = rrand.iRand64HQ()%64*2;
				else
					motifs[s][n] = -1;
			}
			rcout << "Changing drum motif "<< s <<"\n";
		}

		r = rrand.iRand64HQ()%1000;
		if(r<400)
		{
			slots[rrand.iRand64HQ()%(phrase_length/8)] = rrand.iRand64HQ()%4;
			rcout << "Changing drum slot\n";
		}
		else if(r > 500 && r<800)
		{
			int s = rrand.iRand64HQ()%4;
			for(int n=0;n<16;n++)
			{
				if(rrand.iRand64HQ()%2==0 || (n%4)==0)
					motifs[s][n] = rrand.iRand64HQ()%32*4;
				else if(rrand.iRand64HQ()%2==0)
					motifs[s][n] = rrand.iRand64HQ()%64*2;
				else
					motifs[s][n] = -1;
			}
			rcout << "Changing drum motif "<< s <<"\n";
		}

		for(int i=0;i<phrase_length;i++)
		{
			if(drums)
			{
				if(i%2==0)
				{
					if(slots[i/8]>=0 && motifs[slots[i/8]][i%8]>=0)
					{
						Note tnote = notes[i];
						tnote.pan = pan;
						tnote.note=motifs[slots[i/8]][i%8];
						tnote.instrument = instrument;
						tnote.length = 2.0/16.0;
						//tnote.length = 1.0;
						tnote.velocity = 220.0;
						newnotes.push_back(tnote);
					}
				}
			}
			if(sample)
			{
				if(i%2==0)
				{
					if(slots[i/8]>=0 && motifs[slots[i/8]][i%8]>=0)
					{
						Note tnote = notes[i];
						tnote.pan = pan;
						tnote.note=motifs[slots[i/8]][i%8];
						tnote.instrument = instrument;
						tnote.length = 2.0/16.0;
						//tnote.length = 1.0;
						tnote.velocity = 260.0;
						newnotes.push_back(tnote);
					}
				}
			}
		}
	}
};

class Voices
{
public:
	MusicPlayer *mp;
	vector<Voice*> voices;

	int lead;
	int acid;
	int bass;
	int strings;
	int pad;
	int arp;
	int drums;
	int samples;

	Voices()
	{
		drums=arp=lead=acid=bass=strings=pad=0;
		mp=NULL;
	}

	void InitVoices(MusicPlayer *MP, double Tempo, double density)
	{
		for(uint i=0;i<voices.size();i++)
			delete voices[i];
		voices.clear();

		samples=drums=arp=lead=acid=bass=strings=pad=0;
		mp = MP;
		Voice *v = NULL;

		v = new Voice();
		v->lead = true;
		//v->status = 1;
		lead+=v->status;
		v->pan = 0.4;
		v->density = 1.8 * density;
		v->instrument = mp->MakeInstrument(2);
		voices.push_back( v );

		v = new Voice();
		v->lead = true;
		//v->status = 1;
		lead+=v->status;
		v->key=3;
		v->density = 1.5 * density;
		v->pan = 0.6;
		v->instrument = mp->MakeInstrument(0);
		voices.push_back( v );

		v = new Voice();
		v->strings = true;
		//v->status = 1;
		strings+=v->status;
		v->key=7;
		v->pan = 0.5;
		v->instrument = mp->MakeInstrument(5);
		voices.push_back( v );

		v = new Voice();
		v->bass = true;
		//v->status = 1;
		bass+=v->status;
		v->key=-7;
		v->instrument = mp->MakeInstrument(1);
		voices.push_back( v );
		
		v = new Voice();
		v->arp = true;
		//v->status = 1;
		arp+=v->status;
		v->instrument = mp->MakeInstrument(0);
		voices.push_back( v );

		v = new Drummer();
		//v->status=1;
		drums+=v->status;
		v->instrument = mp->MakeBreak("amen.snd", 16.0/(Tempo/60.0)*1000.0, 1.0 );
		//v->instrument = mp->MakeBreak("funky drummer 2.snd", 16.0/(Tempo/60.0)*1000.0, 2.0 );
		voices.push_back(v);

		/*v = new Drummer();
		//v->status=1;
		v->sample=true;
		v->drums=false;
		v->instrument = mp->MakeBreak("second.snd", 16.0/(Tempo/60.0)*1000.0, 1.0 );
		voices.push_back(v);

		v = new Drummer();
		//v->status=1;
		v->sample=true;
		v->drums=false;
		v->instrument = mp->MakeBreak("samples.snd", 0.0, 1.0 );
		voices.push_back(v);*/
	}

	virtual ~Voices()
	{
		for(uint i=0;i<voices.size();i++)
			delete voices[i];
		voices.clear();
	}

	void Play(Note *notes, int phrase_length, RayRand64 &rrand, vector<Note> &newnotes)
	{
		do
		{
			int r = rrand.iRand64HQ()%1000;
			//r=800000;
			if(r>=0 && r<70)
			{
				for(uint i=0;i<voices.size();i++)
				{
					if(voices[i]->drums && voices[i]->status==0)
					{
						voices[i]->status = 1;
						rcout << "Turning on drums\n";
						drums++;
					}
				}
			}
			if(r>=70 && r<100)
			{
				for(uint i=0;i<voices.size();i++)
				{
					if(voices[i]->drums && voices[i]->status==1)
					{
						voices[i]->status = 0;
						rcout << "Turning off drums\n";
						drums--;
					}
				}
			}
			if(r>=100 && r<180)
			{
				for(uint i=0;i<voices.size();i++)
				{
					if(voices[i]->arp)
					{
						if(voices[i]->status == 1)
						{
							voices[i]->status = 0;
							for(uint a=0;a<voices.size();a++)
								if(a!=i && (voices[a]->lead || voices[a]->bass || voices[a]->strings) )
								{
									if(voices[a]->status==0)
									{
										if(voices[a]->lead)
											lead++;
										else if(voices[a]->strings)
											strings++;
										else if(voices[a]->bass)
											bass++;
									}
									voices[a]->status = 1;
								}
								rcout << "Turning off arpeggiator\n";
								arp--;
						}
					}
				}
			}
			if(r>=195 && r<200)
			{
				for(uint i=0;i<voices.size();i++)
				{
					if(voices[i]->arp)
					{
						if(voices[i]->status == 0)
						{
							voices[i]->status = 1;
							for(uint a=0;a<voices.size();a++)
								if(a!=i && voices[a]->drums==false)
									voices[a]->status = 0;
							rcout << "Turning on arpeggiator\n";
							arp++;
						}
					}
				}
			}
			if(r>=200 && r<270)
			{
				for(uint i=0;i<voices.size();i++)
					if(voices[i]->lead)
					{
						if(voices[i]->status)
						{
							if(r>=210)
							{
								voices[i]->status=0;
								lead--;
								rcout << "Turning lead off\n";
							}
						}
						else
						{
							voices[i]->status=1;
							lead++;
							rcout << "Turning lead on\n";
						}
					}
			}
			if(r>=300 && r<370)
			{
				for(uint i=0;i<voices.size();i++)
					if(voices[i]->pad)
					{
						if(voices[i]->status)
						{
							if(r>=310)
							{
								voices[i]->status=0;
								pad--;
								rcout << "Turning pad off\n";
							}
						}
						else
						{
							voices[i]->status=1;
							pad++;
							rcout << "Turning pad on\n";
						}
					}
			}
			if(r>=400 && r<470)
			{
				for(uint i=0;i<voices.size();i++)
					if(voices[i]->bass)
					{
						if(voices[i]->status)
						{
							if(r>=410)
							{
								voices[i]->status=0;
								bass--;
								rcout << "Turning bass off\n";
							}
						}
						else
						{
							voices[i]->status=1;
							bass++;
							rcout << "Turning bass on\n";
						}
					}
			}
			if(r>=500 && r<570)
			{
				for(uint i=0;i<voices.size();i++)
					if(voices[i]->strings)
					{
						if(voices[i]->status)
						{
							if(r>=515)
							{
								voices[i]->status=0;
								strings--;
								rcout << "Turning strings off\n";
							}
						}
						else
						{
							voices[i]->status=1;
							strings++;
							rcout << "Turning strings on\n";
						}
					}
			}
			if(r>=500 && r<600)
			{
				for(uint i=0;i<voices.size();i++)
					if(voices[i]->sample)
						voices[i]->status = voices[i]->status==0;
				rcout << "Toggling samples\n";
			}

		} while(strings==0 && lead==0 && bass==0 && drums==0 && arp==0);

		for(uint i=0;i<voices.size();i++)
			voices[i]->Play(notes,phrase_length,rrand,newnotes);
	}
};

class Motif3
{
public:
	int vels[16];
	unsigned char keys[16];
	int length;

	Motif3(RayRand64 &rrand, int Length)
	{
		length = Length;
		for(int i=0; i<16; i++)
		{
			keys[i] = rrand.iRand64HQ()%6;
		}
		keys[0] = 2;
		for(int i=0; i<16; i++)
		{
			vels[i] = ((i%8==0)*2 + (i%4==0)*2 + (i%2==0))*8 + int(rrand.iRand64HQ()%100)-20;
			vels[i] = min(vels[i], 127);
			vels[i] = max(vels[i], 0);
		}
	}

	/*Motif3(RayRand64 &rrand, int Length, Motif3 &M)
	{
		length = Length;
		for(int i=0; i<16; i++)
		{
			keys[i] = (rrand.iRand64HQ()%6 + M.keys[i])/2;
		}
		keys[0] = 2;
		for(int i=0; i<16; i++)
		{
			vels[i] = ((i%8==0)*2 + (i%4==0)*2 + (i%2==0))*8 + int(rrand.iRand64HQ()%100)-20;
			vels[i] = (min(vels[i], 127) + M.vels[i])/2;
			vels[i] = max(vels[i], 0);
		}
	}*/

	Motif3(RayRand64 &rrand, int Length, Motif3 &M, int option=0)
	{
		if(option&1)
		{
			length = Length;
			for(int i=0; i<16; i++)
			{
				keys[i] = (rrand.iRand64HQ()%6 + M.keys[i/2])/2;
				vels[i] = 0;
			}
			keys[0] = 2;
			for(int i=0; i<16; i+=2)
			{
				vels[i] = ((i%8==0)*2 + (i%4==0)*2 + (i%2==0))*8 + int(rrand.iRand64HQ()%100)-20;
				vels[i] = (min(vels[i], 127) + M.vels[i/2])/2;
				vels[i] = max(vels[i], 0);
			}
		}
		else
		{
			length = Length;
			for(int i=0; i<16; i++)
			{
				keys[i] = (rrand.iRand64HQ()%6 + M.keys[i])/2;
			}
			keys[0] = 2;
			for(int i=0; i<16; i++)
			{
				vels[i] = ((i%8==0)*2 + (i%4==0)*2 + (i%2==0))*8 + int(rrand.iRand64HQ()%100)-20;
				vels[i] = (min(vels[i], 127) + M.vels[i])/2;
				vels[i] = max(vels[i], 0);
			}
		}
	}

	int Apply(Note *notes, int s, int e, unsigned char Key, RayRand64 &rrand, unsigned char damage, int velocity)
	{
		for(int a=0; a< length && s<e; a++, s++)
		{
			//notes[s].note = wnotes[keys[a] + Key];
			notes[s].note = keys[a] + Key;
			int Vel = min(vels[a] + velocity, 127);
			Vel = max(Vel, 0);
			notes[s].velocity = double(Vel);
		}

		return s;
	}
};

class MotifC
{
public:
	vector<Motif3> Motifs;
	int length;

	unsigned char slots[32];
	int keys[32];

	unsigned char GenSlot(RayRand64 &rrand)
	{
		unsigned char r = (unsigned char)(rrand.iRand64HQ()% (Motifs.size()) )+1;
		//if(rrand.iRand64HQ() % 100 < 15)
		//	r=0;
		return r;
	}

	int GenKey(RayRand64 &rrand)
	{
		//return 0;
		return int(rrand.iRand64HQ()%7) -3;
	}

	MotifC()
	{
		length = 0;
		for(int i=0; i<32; i++)
		{
			slots[i] = 0;
		}
	}

	MotifC(RayRand64 &rrand, int Length, int motifs)
	{
		length = Length;
		int c = motifs;
		for(int i=0; i<c; i++)
		{
			Motifs.push_back( Motif3(rrand, 8) );
		}

		for(int i=0; i<c; i++)
			rcout << "Motif "<<i<<", length=="<<Motifs[i].length<<"\n";

		for(int i=0; i<32; i++)
		{
			keys[i] = GenKey(rrand);
		}
		for(int i=0; i<32; i++)
		{
			slots[i] = GenSlot(rrand);
		}
	}

	int Apply(Note *notes, int s, int e, unsigned char Key, RayRand64 &rrand, unsigned char damage, int velocity)
	{
		/*keys[e/16] = */keys[0] = 0;
		for(int o=0,i=0;o<length*16 && s<e; o+=Motifs[0].length, i++)
		{
			//int r = rrand.iRand64HQ()%1800;
			//int r = rrand.iRand64HQ()%1200;
			int r = rrand.iRand64HQ()%800;
			int v = velocity;
			unsigned char d = damage;

			if(slots[i] > Motifs.size())
				slots[i] = GenSlot(rrand);

			/*if(rrand.iRand64HQ()%64==0)
				r = 150;
			else
				r = 1800;*/

			//cout << "r == "<<r<<"\n";
			if(r>=300 && r < 340)
			{
				if(i!=0/* && i!=e/16*/)
				{
					keys[i] = GenKey(rrand);
					rcout << "Changing a key\n";
				}
			}
			else if(r >= 10 && r < 20)
			{
				if(Motifs.size()<6)
				{
					Motifs.push_back( Motif3(rrand, Motifs[slots[i]-1].length, Motifs[slots[i]-1], 0) );
					rcout << "Adding motif ("<<Motifs.size()<<")\n";
				}
			}
			else if(r >= 20 && r < 30)
			{
				if(Motifs.size()<6)
				{
					Motifs.push_back( Motif3(rrand, Motifs[slots[i]-1].length, Motifs[slots[i]-1], 1) );
					rcout << "Adding stretched motif ("<<Motifs.size()<<")\n";
				}
			}
			else if(r >= 30 && r < 40)//a little more likely than adding one(not anymore...)
			{
				if(Motifs.size()>3)
				{
					Motifs.pop_back();
					if(slots[i] > Motifs.size())
						slots[i] = GenSlot(rrand);
					rcout << "Removing motif ("<<Motifs.size()<<")\n";
				}
			}
			else if(r > 50 && r < 100)
			{
				slots[i] = GenSlot(rrand);
				rcout << "Changing a slot\n";
			}
			//else if(r < 200)
			else if(r >= 100 && r < 200)
			{
				if(slots[i]>0)
				{
					Motifs[slots[i]-1] = Motif3(rrand, Motifs[slots[i]-1].length, Motifs[slots[i]-1]);
					rcout << "Changing a motif\n";
				}
			}
			/*else if(r < 300)
			{
				rcout << "This jam is amplified!\n";
				d = 1;
				v += 20;
			}*/

			if(r > 400 && r < 500)
			{
				Motif3 m(rrand, 4);
				if(slots[i] != 0)
					m = Motif3(rrand, Motifs[slots[i]-1].length, Motifs[slots[i]-1]);
				else
				{
					int s = rrand.iRand64HQ()%Motifs.size();
					m = Motif3(rrand, Motifs[s].length, Motifs[s]);
				}
				s=m.Apply(notes, s, e, Key + keys[i], rrand, d, v);
				rcout << "Using a modified motif\n";
			}
			else
			{
				if(slots[i]>0)
				{
					if(r > 500 && r < 600)
					{
						rcout << "Shorts\n";
						s=Motifs[slots[i]-1].Apply(notes, s, s+Motifs[slots[i]-1].length/2, Key + keys[i], rrand, d, v);
						s=Motifs[slots[i]-1].Apply(notes, s, s+Motifs[slots[i]-1].length/2, Key + keys[i], rrand, d, v);
					}
					else
						s=Motifs[slots[i]-1].Apply(notes, s, e, Key + keys[i], rrand, d, v);
				}
				else
					s+=Motifs[0].length;
			}
		}

		return s;
	}
};

class NormalSong3 : public Song
{
public:
	unsigned __int64 seed;
	MotifC Motifs;
	int PhraseLength;
	double density;
	Voices voices;

	NormalSong3(MusicPlayer *MP) : Song(MP)//, voices(MP)// : Motifs(rrand, 8, 2)
	{
		for(int i=1;i<11;i++)
		{
			for(int a=0;a<7;a++)
			{
				wnotes[a+i*7] = wnotes[a] + i*12;
			}
		}
		PhraseLength = 8;
		Tempo = 100.0;
		density = 0.7;

		seed = rrand.newseed.newseed64;
		//seed = 5001116690293865754;
		//seed = 9851927168813245727;
		rrand.newseed.newseed64 = seed;

		//Key = 48 + rrand.iRand64HQ()%16;
		Key = 28 + rrand.iRand64HQ()%10;

		Tempo = double(rrand.iRand64HQ()% 500)/10.0 + 70.0;
		density = 0.4 + double(rrand.iRand64HQ()%300)/1000.0;
		PhraseLength = rrand.iRand64HQ()%6 + 3;
		Tempo *= 1.5;
		//Tempo = 300.0;
		//PhraseLength = 8;

		//Tempo *= 2.4;

		//PhraseLength = 10;

		Motifs = MotifC(rrand, PhraseLength, rrand.iRand64HQ()%2+2);

		Length = max(PhraseLength,8)*12;
		//Length = max(PhraseLength,8)*20;
		Length += (double)(rrand.iRand64HQ()%100);
		//Length *= 2;

		//Length = 100000.0;
		Length = double(int(Length));
		Length -= double(int(Length)%PhraseLength);

		voices.InitVoices(MP, Tempo, density);

		rcout << "seed == "<<seed<<"\n"
			<< "length == "<<Length<<", PhraseLength == "<<PhraseLength<<", density == "<<density<<", Tempo == "<<Tempo
			<< ", Motifs == "<<Motifs.Motifs.size() << ", Min Velocity == "<< int(127.0 - density*127.0) <<", Key == "<< int(wnotes[Key]) << "\n-------------------------------\n\n";
	}

	void NewNote(Note &note)
	{
		AddNote(note.time, note.length, note.note, double(note.velocity)/127.0, note.instrument, note.pan, note.mod);
	}

	void Gen()
	{
		int p = int(Pos);
		double GenPos = double(p);

		rrand.newseed.newseed64 = (p/PhraseLength +1)*seed;

		LastGen = double(p+PhraseLength);
		rcout << "\n---Gen bars "<<p<<" to "<< int(LastGen) << "\n";//<<", seed == "<< rrand.newseed.newseed64 <<"\n";

		Note notes[1024];
		for(int i=0;i<PhraseLength*16;i++)
		{
			notes[i].length = 0.005;
			notes[i].time = double(i)/16.0 + GenPos;
			notes[i].instrument = 0;
			notes[i].velocity = 127.0;
			notes[i].note = 100+(i%4)*2;
		}

		for(int i=0; i<PhraseLength*16; )
		{
			i= Motifs.Apply(notes, i, PhraseLength*16, Key, rrand, 0, 0);
		}

		vector<Note> newnotes;
		voices.Play(notes, PhraseLength*16, rrand, newnotes);
		for(uint i=0;i<newnotes.size();i++)
			NewNote(newnotes[i]);
		return;
	}
};

Song *NewSong(MusicPlayer *MP)
{
	return new NormalSong3(MP);
}
