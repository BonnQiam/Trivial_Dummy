#ifndef Rectangle_hpp
#define Rectangle_hpp

#include "Coor.hpp"

/***********************************************
* Rectangle Definition
* width: rectangle width
* height: rectangle height
* bl: bottom left coordinate of this rectangle
* tr: top right coordinate of this rectangle
************************************************/ 
template <typename T>
struct Rect
{
private:/*****/
    Coor<T> bl, tr;
    T width, height;

    void resetWH();
public:
    Rect(const T x=T(), const T y=T(), const T w=T(), const T h=T()) : bl(x, y), tr(x+w, y+h), width(w), height(h) {}
    Rect(const Coor<T>& nbl, const Coor<T>& ntr) : bl(nbl), tr(ntr) { resetWH();}

    inline const Coor<T>& getBL() const { return bl; }
    inline const Coor<T>& getTR() const { return tr; }
    inline const Coor<T>& getTL() const { return Coor<T>(bl.getX(), tr.getY()); }
    inline const Coor<T>& getBR() const { return Coor<T>(tr.getX(), bl.getY()); }

    inline const T& getW() const 		{ return width; }
    inline const T& getH() const		{ return height; }

    void setBL(const T x, const T y);
    void setBL(const Coor<T>& nbl);

    void setTR(const T x, const T y);
    void setTR(const Coor<T>& ntr);

    void set(const T x1, const T y1, const T x2, const T y2);
    void setW(const T w, const T h);
    void setH(const T w, const T h);

    void blAddToX(const T val);
    void blAddToY(const T val);
    
    void trAddToX(const T val);
    void trAddToY(const T val);
    
    void shiftX(const T val);
    void shiftY(const T val);

    T Area() const { return width * height; }
};

template <typename T>
void Rect<T>::resetWH()
{
    width  = tr.getX() - bl.getX();
    height = tr.getY() - bl.getY();
}

template <typename T>
void Rect<T>::setBL(const T x, const T y)
{
    bl.set(x, y);
    resetWH();
}

template <typename T>
void Rect<T>::setBL(const Coor<T>& nbl)
{
    bl = nbl;
    resetWH();
}

template <typename T>
void Rect<T>::setTR(const T x, const T y)
{
    tr.set(x, y);
    resetWH();
}

template <typename T>
void Rect<T>::setTR(const Coor<T>& ntr)
{
    tr = ntr;
    resetWH();
}

template <typename T>
void Rect<T>::set(const T x1, const T y1, const T x2, const T y2)
{
    bl.set(x1, y1);
    tr.set(x2, y2);
    resetWH();
}

template <typename T>
void Rect<T>::setW(const T w, const T h)
{
    width = w;
}

template <typename T>
void Rect<T>::setH(const T w, const T h)
{
    height = h;
}

template <typename T>
void Rect<T>::blAddToX(const T val)
{
    bl.addToX(val);
    width -= val;
}

template <typename T>
void Rect<T>::blAddToY(const T val)
{
    bl.addToY(val);
    height -= val;
}

template <typename T>
void Rect<T>::trAddToX(const T val)
{
    tr.addToX(val);
    width += val;
}

template <typename T>
void Rect<T>::trAddToY(const T val)
{
    tr.addToY(val);
    height += val;
}

template <typename T>
void Rect<T>::shiftX(const T val)
{
    bl.addToX(val);
    tr.addToX(val);
}

template <typename T>
void Rect<T>::shiftY(const T val)
{
    bl.addToY(val);
    tr.addToY(val);
}

// return intersection reactangle of two Rectangle
template <typename T>
Rect<T> Rectangle_intersection(const Rect<T>& r1, const Rect<T>& r2)
{
    // Check intersection
    Coor<T> tl(std::max(r1.getTL().getX(), r2.getTL().getX()), 
                std::min(r1.getTL().getY(), r2.getTL().getY()));
                
    Coor<T> br(std::min(r1.getBR().getX(), r2.getBR().getX()), 
                std::max(r1.getBR().getY(), r2.getBR().getY()));

    //check if the intersection is valid
    if(tl.getX() > br.getX() || tl.getY() < br.getY())
        return Rect<T>(Coor<T>(0, 0), Coor<T>(0, 0));

    // return intersection rectangle
    Coor<T> bl(tl.getX(), br.getY());
    Coor<T> tr(br.getX(), tl.getY());
    
    return Rect<T>(bl, tr);
}

#endif