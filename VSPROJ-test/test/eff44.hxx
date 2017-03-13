#include <iostream>
#include <boost/scoped_array.hpp>
using namespace std;

//1.
//template<typename T> //与尺寸无关的baseclass ，
//class SquareMatrixBase 
//{ //用于正方矩阵
//protected:
//	void invert(std::size_t matrixSize) //以给定的尺寸求逆矩阵
//	{
//		cout << "invert:" << matrixSize << endl;
//	}
//};

//2.
template<typename T>
class SquareMatrixBase 
{
protected:
	SquareMatrixBase(std::size_t n, T* pMem)
		:size(n) , pData(pMem) { }
	void setDataPtr(T* ptr) { pData = ptr; }
	void invert(std::size_t matrixSize) //以给定的尺寸求逆矩阵
	{
		cout << "invert:" << matrixSize << endl;
	}
private:
	std::size_t size;
	T* pData;
};

//1.
template<typename T, std::size_t n>
class SquareMatrix: private SquareMatrixBase<T> 
{
private:
	using SquareMatrixBase<T>::invert; //避免遮掩base 版的invert; 见条款33
public:
	SquareMatrix()
		:SquareMatrixBase<T>(n,data) { }
	void invert( ) { this->invert(n); }
private:
	T data[n*n];
};

//2.
template<typename T, std::size_t n>
class SquareMatrixNew: private SquareMatrixBase<T> 
{
private:
	using SquareMatrixBase<T>::invert; //避免遮掩base 版的invert; 见条款33
public:
	SquareMatrixNew()
		:SquareMatrixBase<T>(n,NULL)
		,pData(new T[n*n])
	{
		this->setDataPtr(pData.get());
	}
	void invert( ) { this->invert(n); }
private:
	boost::scoped_array<T> pData;
};


int main()
{
	SquareMatrix<double,5> sqmatrix1;
	sqmatrix1.invert();

	SquareMatrixNew<double,5> sqmatrix2;
	sqmatrix2.invert();

	return 0;
}