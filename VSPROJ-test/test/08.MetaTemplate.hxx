template<bool b, class X, class Y>
struct if_ {
	typedef X type; // use X if b is true
};

template<class X, class Y>
struct if_<false,X,Y> {
	typedef Y type; // use Y if b is false
};
 
class Foobar
{
};

class Foo
{
};

class Bar
{
};

typedef if_<1, Foo, Bar>::type type;
//1. type 类型为Foo
//0. type 类型为Bar

int main()
{
	type a;

	return 0;
}