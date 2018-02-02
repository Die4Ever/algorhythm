
class MusicPlayer
{
public:
	virtual double GetCounter()=0;//returns milliseconds+buffer size, aka Gen upto time
	virtual void AddNote(double time, double length, unsigned char note, double velocity, int instrument, double pan, double mod)=0;
	virtual ~MusicPlayer(){};
	virtual int Play()=0;
	virtual int MakeInstrument(unsigned __int64 tags)=0;
	virtual int MakeBreak(char *filename, double length, double volume)=0;
};

class Instrument;

class MusicSynth : public MusicPlayer
{
public:
	RaySound rs;
	vector<Instrument*> instruments;
	int InstrumentID;
	unsigned __int64 time;
	ofstream outputfile;

	virtual double GetCounter();//returns milliseconds+buffer size, aka Gen upto time
	virtual void AddNote(double start_time, double length, unsigned char note, double velocity, int instrument, double pan, double mod);
	virtual int Play();
	virtual ~MusicSynth();
	virtual int MakeInstrument(unsigned __int64 tags);
	virtual int MakeBreak(char *filename, double length, double volume);
	MusicSynth();
};

