
class MidiEvent
{
public:
	double time;
	int instrument;

	unsigned char status;
	unsigned char note;
	unsigned char velocity;

	MidiEvent(double Time, int Instrument, unsigned char Status, unsigned char Note, unsigned char Velocity)
	{
		time = Time;
		instrument = Instrument;
		status = Status;
		note = Note;
		velocity = Velocity;
	}

	int SortComp(MidiEvent *objB)
	{
		if( time < objB->time )
			return -1;
		else if( time > time )
			return 1;
		return 0;
	}

	int SortComp(double objB)
	{
		if( time < objB )
			return -1;
		else if( time > objB )
			return 1;
		return 0;
	}
};

class Song
{
public:
	MusicPlayer *mp;
	//AVLTree2<MidiEvent, double, RayHeapList> Events;
	RayRand64 rrand;
	unsigned char Key;

	double Length;
	double Pos;
	double Tempo;
	double LastGen;

	Song(MusicPlayer *MP)
	{
		mp=MP;
		rrand.SetStrongSeed( time(0) );
		Key = 54;
		Pos = 0.0;
		LastGen = 0.0;
		Length = 1000.0;
		Tempo = 120.0;
	}

	virtual ~Song()
	{
	}

	void AddNote(double pos, double len, unsigned char key, double velocity, int instrument, double pan, double mod)
	{
		mp->AddNote(pos/(Tempo/4/60000.0),len/(Tempo/4/60000.0), key, velocity, instrument, pan, mod);
	}

	virtual void Gen()
	{
		double i = double(int(LastGen));
		AddNote(i, 0.2, 60, 1.0, 0, 0.5, 0.0);
		AddNote(i+0.25, 0.2, 50, 1.0, 0, 0.5, 0.0);
		AddNote(i+0.5, 0.2, 40, 1.0, 0, 0.5, 0.0);
		AddNote(i+0.75, 0.2, 30, 1.0, 0, 0.5, 0.0);
		LastGen = LastGen+1.0;
	}

	virtual void Play()
	{
		do
		{
			if(LastGen <= Pos)
				Gen();

			//Sleep(1);
			mp->Play();
			Pos = (Tempo/4/60000.0)*mp->GetCounter();
		} while( Pos < Length );

		//cout << "Fin\n";
		mp->Play();
		mp->Play();
		mp->Play();
		mp->Play();
		mp->Play();
		mp->Play();
		mp->Play();
		mp->Play();
		//cout << "K, done, Pos == "<<Pos<<"\n";
	}

};

Song *NewSong(MusicPlayer *MP);
