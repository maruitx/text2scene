#ifndef COLOR_H
#define COLOR_H

#include "Headers.h"
#include <QColor>

class Color
{

public:
   
    Color();
    Color(float r, float g, float b, float a);
    Color(float r, float g, float b);
    Color(int r, int g, int b);
    Color(int r, int g, int b, int a);
    Color(const Color &c);
    Color(const float *c);
    Color(const QColor &c);
    ~Color();

    void set(float r, float g, float b, float a);

    Color &get();
    float *getArray();

    void getRGBA(float &r, float &g, float &b, float &a);
    void getRGBA(int &r, int &g, int &b, int &a);
    QColor getQColor();


    void print();
    void clamp();
    float luminance();
    void invert();
    Color inverted();

    //Assignment
    Color &operator =  (const Color &c);
    Color &operator += (const Color &c);
    Color &operator += (float s);
    Color &operator -= (const Color &c);
    Color &operator -= (float s);
    Color &operator *= (const Color &c);
    Color &operator *= (float s);    
    Color &operator /= (const Color &c);
    Color &operator /= (float s);
	  
    //Arithmetic
    Color operator + (const Color &c) const;
    Color operator + (float s) const;	
    Color operator - (const Color &c) const;
    Color operator - (float s) const;
    Color operator - () const;
	Color operator * (const Color &c) const;
    Color operator * (float s) const;
    Color operator / (const Color &c) const;
    Color operator / (float s) const;

    //Comparison
    bool operator == (const Color &c);
    bool operator != (const Color &c);
    bool operator <= (const Color &c);
    bool operator <  (const Color &c);
    bool operator >= (const Color &c);
    bool operator >  (const Color &c);	

   float r;
   float g;
   float b; 
   float a;

   float color[4];

private:
   
};


#endif
