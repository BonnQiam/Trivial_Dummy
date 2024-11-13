/*
1. template class Coor
2. 基于 Coor 衍生出的运算符重载
*/

#ifndef Coor_hpp
#define Coor_hpp

#include <climits>
#include <ostream>
#include <exception>
#include <cmath>

#include <iostream>

#define log 0

typedef std::exception NullPtrException;

/// @brief Template class Coor
/// @tparam coor_t 
template <typename coor_t>
class Coor
{
private:
	coor_t x, y;

public:
	static const int INF = INT_MAX;
	static const int NINF = INT_MIN;

	explicit Coor(coor_t a=0, coor_t b=0) : x(a), y(b) {}
	Coor(const Coor<coor_t>& rhs) : x(rhs.x), y(rhs.y) {}
	virtual ~Coor() {}

	// setter
    void set(coor_t a=0, coor_t b=0);
    void setX(coor_t new_x);
    void setY(coor_t new_y);
	// getter
	coor_t getX() const;
	coor_t getY() const;
	// add value
	void addToX(coor_t value);
	void addToY(coor_t value);

	Coor& operator-= (const Coor<coor_t>& right);
	Coor& operator+= (const Coor<coor_t>& right);
    Coor& operator+= (const coor_t& value);
	Coor& operator-= (const coor_t& value);
};

template <typename coor_t>
void Coor<coor_t>::set(coor_t a, coor_t b ) { x = a, y = b; }

template <typename coor_t>
void Coor<coor_t>::setX(coor_t new_x) { x = new_x; }

template <typename coor_t>
void Coor<coor_t>::setY(coor_t new_y) { y = new_y; }

template <typename coor_t>
coor_t Coor<coor_t>::getX() const { return x; } //! const 置于函数后侧是什么意思

template <typename coor_t>
coor_t Coor<coor_t>::getY() const { return y; }

template <typename coor_t>
void Coor<coor_t>::addToX(coor_t value) { x += value; }

template <typename coor_t>
void Coor<coor_t>::addToY(coor_t value) { y += value; }

template <typename coor_t>
Coor<coor_t>& Coor<coor_t>::operator-= (const Coor<coor_t>& right)
{
	this->x -= right.x;
	this->y -= right.y;
	return *this;
}

template <typename coor_t>
Coor<coor_t>& Coor<coor_t>::operator+= (const Coor<coor_t>& right)
{
	this->x += right.x;
	this->y += right.y;
	return *this;
}

template <typename coor_t>
Coor<coor_t>& Coor<coor_t>::operator+= (const coor_t& value)
{
	this->x += value;
	this->y += value;
	return *this;
}

template <typename coor_t>
Coor<coor_t>& Coor<coor_t>::operator-= (const coor_t& value)
{
	this->x -= value;
	this->y -= value;
	return *this;
}

/*************** operator overloading for Coor *********************/

/** overload stream operator for Coor **/
template <typename coor_t>
std::ostream& operator<< (std::ostream &out, const Coor<coor_t> &coor)
{
#if log
	std::cout << "You are using the operator<< overload" << std::endl;
#endif
	out << '(';
	if(coor.getX() == Coor<coor_t>::INF) out << "inf";
	else if(coor.getX() == Coor<coor_t>::NINF) out << "-inf";
	else out << coor.getX();
	out << ", ";
	if(coor.getY() == Coor<coor_t>::INF) out << "inf";
	else if(coor.getY() == Coor<coor_t>::NINF) out << "-inf";
	else out << coor.getY();
	out << ')';
	return out;
}

template <typename coor_t>
inline Coor<coor_t> operator- (const Coor<coor_t>& lhs, const Coor<coor_t>& rhs)
{
#if log
	std::cout << "You are using the operator- overload" << std::endl;
#endif
	Coor<coor_t> temp(lhs);
	temp -= rhs;
	return temp;
}


template <typename coor_t>
inline Coor<coor_t> operator+ (const Coor<coor_t>& lhs, const Coor<coor_t>& rhs)
{
#if log
	std::cout << "You are using the operator+ overload" << std::endl;
#endif
	Coor<coor_t> temp(lhs);
	temp += rhs;
	return temp;
}


template <typename coor_t>
inline Coor<coor_t> operator- (Coor<coor_t>&& lhs, const Coor<coor_t>& rhs)
{
#if log
	std::cout << "You area using the operator- overload with rvalue reference" << std::endl;
#endif
	lhs -= rhs;
	return lhs;
}


template <typename coor_t>
inline Coor<coor_t> operator+ (Coor<coor_t>&& lhs, const Coor<coor_t>& rhs)
{
#if log
	std::cout << "You area using the operator+ overload with rvalue reference" << std::endl;
#endif
	lhs += rhs;
	return lhs;
}


template <typename coor_t>
inline Coor<coor_t> operator- (const Coor<coor_t>& lhs, const coor_t& val)
{	
#if log
	std::cout << "You are using the operator- overload with scalar value" << std::endl;
#endif
	Coor<coor_t> temp(lhs);
	temp -= val;
	return temp;
}

template <typename coor_t>
inline Coor<coor_t> operator+ (const Coor<coor_t>& lhs, const coor_t& val)
{	
#if log
	std::cout << "You are using the operator+ overload with scalar value" << std::endl;
#endif
	Coor<coor_t> temp(lhs);
	temp += val;
	return temp;
}


template <typename coor_t>
inline Coor<coor_t> operator- (Coor<coor_t>&& lhs, const coor_t& rhs)
{	
#if log
	std::cout << "You are using the operator- overload with scalar value and rvalue reference" << std::endl;
#endif
	lhs -= rhs;
	return lhs;
}


template <typename coor_t>
inline Coor<coor_t> operator+ (Coor<coor_t>&& lhs, const coor_t& rhs)
{	
#if log
	std::cout << "You are using the operator+ overload with scalar value and rvalue reference" << std::endl;
#endif
	lhs += rhs;
	return lhs;
}


template <typename coor_t>
inline bool operator< (const Coor<coor_t>& lhs, const Coor<coor_t>& rhs)
{
#if log
	std::cout << "You are using the operator< overload" << std::endl;
#endif
    return ((lhs.getX() < rhs.getX()) && (lhs.getY() < rhs.getY()));
}


template <typename coor_t>
inline bool operator> (const Coor<coor_t>& lhs, const Coor<coor_t>& rhs)
{
#if log
	std::cout << "You are using the operator> overload" << std::endl;
#endif 

    return ((lhs.getX() > rhs.getX()) && (lhs.getY() > rhs.getY()));
}


template <typename coor_t>
inline bool operator== (const Coor<coor_t>& lhs, const Coor<coor_t>& rhs)
{
#if log
	std::cout << "You are using the operator== overload" << std::endl;
#endif

	if(lhs.getX() != rhs.getX()) { return false; }
	if(lhs.getY() != rhs.getY()) { return false; }
	return true;
}


template <typename coor_t>
inline bool operator!= (const Coor<coor_t>& lhs, const Coor<coor_t>& rhs)
{
#if log
	std::cout << "You are using the operator!= overload" << std::endl;
#endif
	if(lhs == rhs) return false;
	return true;
}

/** specialization version of overloaded operators of Coor **/

inline bool equal(const double a, const double b)
{
	#define EPS 1e-9
	return fabs(a-b)<EPS ? true : false;
}

template <>
inline bool operator< (const Coor<double>& lhs, const Coor<double>& rhs)
{
    if (equal(lhs.getX(), rhs.getX())) { return (lhs.getX() < rhs.getX()); }
    if (equal(lhs.getY(), rhs.getY())) { return (lhs.getY() < rhs.getY()); }
    return ((lhs.getX() < rhs.getX()) && (lhs.getY() < rhs.getY()));
}


/** specialization version of overloaded operators of Coor **/
template <>
inline bool operator> (const Coor<double>& lhs, const Coor<double>& rhs)
{
    if (equal(lhs.getX(), rhs.getX())) { return (lhs.getX() > rhs.getX()); }
    if (equal(lhs.getY(), rhs.getY())) { return (lhs.getY() > rhs.getY()); }
    return ((lhs.getX() > rhs.getX()) && (lhs.getY() > rhs.getY()));
}


template <>
inline bool operator== (const Coor<double>& lhs, const Coor<double>& rhs)
{
	if(!equal(lhs.getX(), rhs.getX())) { return false; }
	if(!equal(lhs.getY(), rhs.getY())) { return false; }
	return true;
}

/******************************************************************************
 * isLef() operation
*/
template <typename T>
double isLeft(Coor<T> a, Coor<T> b, Coor<T> c){
	return (b.getX() - a.getX()) * (c.getY() - a.getY()) - (c.getX() - a.getX()) * (b.getY() - a.getY());
}

#endif