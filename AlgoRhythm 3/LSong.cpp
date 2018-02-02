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

RayRand64 rrand;
#include <LSystem.h>
extern unsigned char wnotes[7*11];

struct BGThreadInfo
{
	CriticalSection *cs;
	LSystem *lsys;
	string initial;

	BGThreadInfo()
	{
		cs=NULL;
		lsys=NULL;
	}

	BGThreadInfo(CriticalSection *CS, LSystem *LSYS, string Initial)
	{
		cs=CS;
		lsys=LSYS;
		initial = Initial;
	}
};

int Background(BGThreadInfo bgtinf)
{
	LSystem *lsys=bgtinf.lsys;
	CriticalSection &cs = *bgtinf.cs;

	while(1)
	{
		string r;
		cin >> r;
		cs.lock();
		if(lsys->rules.size()>3)
		{
			for(uint i=1;i<lsys->rules.size();i++)
				lsys->rules[i-1]=lsys->rules[i];
			lsys->rules.pop_back();
		}
		lsys->AddRule(r);
		cout << "added rule\n";
		cout << "rules -\n";
		uint len = lsys->state.length();
		if(len>9000)
			lsys->state=bgtinf.initial;
		for(uint i=0;i<lsys->rules.size();i++)
			cout << "\t"<<lsys->rules[i].predecessor<<"="<<lsys->rules[i].successor<<"="<<lsys->rules[i].prob<<"\n";
		cout<<"\n";
		lsys->Proc();
		cout << lsys->state << "\n\n";
		cout << "state length is "<<len<<", will reset at over 9000\n\n";
		cs.unlock();
	}
	return 0;
}

struct SongProps
{
	double pos;
	unsigned char key;
};

class LSong : public Song
{
public:
	LSystem lsys;
	CriticalSection cs;
	Thread<BGThreadInfo> bg;

	LSong(MusicPlayer *MP) : Song(MP), lsys("sf+sf--sf-s", "")
	{
		for(int i=1;i<11;i++)
		{
			for(int a=0;a<7;a++)
			{
				wnotes[a+i*7] = wnotes[a] + i*12;
			}
		}
		Length=9999999999.0;
		cout << "Enter tempo: ";
		cin >> Tempo;

		cout << "\n\nEnter intial state, example-\nsf+sf--sf-s\n";
		//cout << "sf+sf--sf-s intro sf+sf--sf-s chorus sf+sf--sf-s bridge sf+sf--sf-s chorus sf+sf--sf-s outro\n";
		string init;
		cin >> init;
		lsys.state=init;

		cout << lsys.state << "\n\n";
		cout << "Enter a rule, for example\ns=(++sf---s---sf++sf++lff)=0.5;\nWhere the first s is what to replace, between the equal signs is what to replace it with, and the number on the end being the probability of doing the replacement(0.0 to 1.0), s means short note, + means increase note, - means decrease note, ( means store note, ) means restore note, f means move the note insertion point forwards, b means move the note insertion point backwards\nTo reprocess the current song without adding a rule just type a and hit enter\n\n";

		bg.StartThread(Background, BGThreadInfo(&cs, &lsys, init));
	}

	virtual void Gen()
	{
		//lsys.Proc();

		LastGen = double(int(LastGen));
		//AddNote(LastGen, 0.4, wnotes[40], 1.0, 0, 0.5, 0.0);
		//AddNote(LastGen+0.5, 0.4, wnotes[41], 1.0, 0, 0.5, 0.0);

		cs.lock();
		string state=lsys.state;
		cs.unlock();

		//rcout << "Playing " << state << "\n\n";

		SongProps sp;
		sp.pos=0.0;
		sp.key=40;
		vector<unsigned char> KeyStack;
		vector<SongProps> SongStack;

		for(uint i=0;i<state.length();i++)
		{
			if(state[i]=='s')
			{
				AddNote(LastGen+sp.pos, 0.2, wnotes[sp.key], 1.0, 0, 0.5, 0.0);
			}
			else if(state[i]=='l')
			{
				AddNote(LastGen+sp.pos, 0.4, wnotes[sp.key], 1.0, 0, 0.5, 0.0);
			}
			else if(state[i]=='f')
			{
				sp.pos+=0.125;
			}
			else if(state[i]=='b')
			{
				sp.pos-=0.125;
			}
			else if(state[i]=='+')
			{
				sp.key++;
			}
			else if(state[i]=='-')
			{
				sp.key--;
			}
			else if(state[i]=='(')
			{
				KeyStack.push_back(sp.key);
			}
			else if(state[i]==')')
			{
				if(KeyStack.size()>0)
				{
					sp.key=KeyStack.back();
					KeyStack.pop_back();
				}
			}
			else if(state[i]=='[')
			{
				SongStack.push_back(sp);
			}
			else if(state[i]=']')
			{
				if(SongStack.size()>0)
				{
					sp=SongStack.back();
					SongStack.pop_back();
				}
			}
		}

		LastGen = double(int(LastGen+sp.pos)+1);
	}
};

/*Song *NewSong(MusicPlayer *MP)
{
	return new LSong(MP);
}*/