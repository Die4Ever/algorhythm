template<unsigned int SIZE>
class RaySine
{
public:
	int size;
	int sizem1;
	double dsize;
	double table[SIZE+1];
	//double step;

	RaySine()
	{
		size = SIZE;
		//table = new double[size+1];

		dsize = (double)(size);//-1.0;
		double d=0.0;
		for(int i=0;i<SIZE;i++)
		{
			table[i] = sin( (d/dsize) *6.283185307179586476925286766559 );
			d+=1.0;
		}
		table[size]=table[0];
		//step = 1.0/double(Size);
		dsize /= 6.283185307179586476925286766559;
		sizem1=size-1;
	}

	~RaySine()
	{
		//delete[] table;
	}

	double Lookup(double d)
	{
		double d1 = d*dsize+0.5;
		unsigned int i = (unsigned int)(d1)&sizem1;
		//i&=sizem1;
		//double ret;
		//d1 = sin(d);
		return table[i];
		//return Interpolate<double>(table[i], table[i+1], fmod(d1, 1.0) );
	}

	double operator()(double d)
	{
		double d1 = d*dsize+0.5;
		unsigned int i = (unsigned int)(d1)&sizem1;
		return table[i];
	}
};
