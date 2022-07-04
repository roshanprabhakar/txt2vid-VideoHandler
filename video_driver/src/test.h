class Mat
{
	int width, height;
};

template <typename T>
class Mat_ : public Mat 
{
	T *buffer;
};


enum EnumType1 : char {
	MASK1 = 'a'
};

enum EnumType2 : char {
	MASK1 = 'b',
	MASK2 = 'c'
};

struct Foo 
{
	int a;

	static int myStatic = 12;
};

void foo() 
{
	Foo f;
	f.a;

	Foo::myStatic;
}
