#include "Color.h"

Color::Color() 
: r(0.0f),
  g(0.0f),
  b(0.0f),
  a(0.0f)
{
}

Color::Color(float r, float g, float b, float a)
{
   this->r = r;
   this->g = g;
   this->b = b;
   this->a = a;
}

Color::Color(float r, float g, float b)
{
   this->r = r;
   this->g = g;
   this->b = b;
   this->a = 1.0f;
}

Color::Color(int r, int g, int b, int a)
{
   this->r = r / 255.0f;
   this->g = g / 255.0f;
   this->b = b / 255.0f;
   this->a = a / 255.0f;
}

Color::Color(int r, int g, int b)
{
   this->r = r / 255.0f;
   this->g = g / 255.0f;
   this->b = b / 255.0f;
   this->a = 1.0f;
}

Color::Color(const Color &c)
{
   this->r = c.r;
   this->g = c.g;
   this->b = c.b;
   this->a = c.a;
}

Color::Color(const float *c)
{
   this->r = c[0];
   this->g = c[1];
   this->b = c[2];
   this->a = 1.0f;
}

Color::Color(const QColor &c)
{
    this->r = (float)c.red() / 255.0f;
    this->g = (float)c.green() / 255.0f;
    this->b = (float)c.blue() / 255.0f;
    this->a = (float)c.alpha() / 255.0f;
}

Color::~Color() 
{
}

void Color::getRGBA(float &r, float &g, float &b, float &a)
{
    r = this->r;
    g = this->g;
    b = this->b;
    a = this->a;
}

void Color::getRGBA(int &r, int &g, int &b, int &a)
{
    r = this->r * 255;
    g = this->g * 255;
    b = this->b * 255;
    a = this->a * 255;
}

QColor Color::getQColor()
{
    int ir = this->r * 255;
    int ig = this->g * 255;
    int ib = this->b * 255;
    int ia = this->a * 255;

    QColor c(ir, ig, ib, ia);

    return c;
}

void Color::set(float r, float g, float b, float a)
{
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}

Color &Color::get()
{
    return *this;
}

float *Color::getArray()
{
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;

    return color;
}

void Color::print()
{
    printf("r: %f, g: %f, b: %f, a: %f\n", r, g, b, a);
}

void Color::clamp()
{
   this->r = g_max<float>(0.0f, g_min<float>(this->r, 1.0f));
   this->g = g_max<float>(0.0f, g_min<float>(this->g, 1.0f));
   this->b = g_max<float>(0.0f, g_min<float>(this->b, 1.0f));
   this->a = g_max<float>(0.0f, g_min<float>(this->a, 1.0f));
}

Color Color::inverted()
{
    Color c;

    c.r = 1.0f - this->r;
    c.g = 1.0f - this->g;
    c.b = 1.0f - this->b;
    c.a = 1.0f - this->a;

    return c;
}

void Color::invert()
{
    this->r = 1.0f - this->r;
    this->g = 1.0f - this->g;
    this->b = 1.0f - this->b;
    this->a = 1.0f - this->a;
}

float Color::luminance()
{
    float max = g_max<float>(g_max<float>(r, g), b);
    float min = g_min<float>(g_min<float>(r, g), b);

    return (min + max) * 0.5;
}

Color &Color::operator =(const Color &c)
{
   this->r = c.r;
   this->g = c.g;
   this->b = c.b;
   this->a = c.a;

   return *this;
}

Color &Color::operator +=(const Color &c)
{
   this->r += c.r;
   this->g += c.g;
   this->b += c.b;
   this->a += c.a;

   return *this;
}

Color &Color::operator +=(float s)
{
   this->r += s;
   this->g += s;
   this->b += s;
   this->a += s;

   return *this;
}

Color &Color::operator -=(const Color &c)
{
   this->r -= c.r;
   this->g -= c.g;
   this->b -= c.b;
   this->a -= c.a;

   return *this;
}

Color &Color::operator -=(float s)
{
   this->r -= s;
   this->g -= s;
   this->b -= s;
   this->a -= s;

   return *this;
}

Color &Color::operator *=(const Color &c)
{
   this->r *= c.r;
   this->g *= c.g;
   this->b *= c.b;
   this->a *= c.a;

   return *this;
}

Color &Color::operator *=(float s)
{
   this->r *= s;
   this->g *= s;
   this->b *= s;
   this->a *= s;

   return *this;
}

Color &Color::operator /=(const Color &c)
{
   this->r /= c.r;
   this->g /= c.g;
   this->b /= c.b;
   this->a /= c.a;
   
   return *this;
}

Color &Color::operator /=(float s)
{
   this->r /= s;
   this->g /= s;
   this->b /= s;
   this->a /= s;

   return *this;
}

Color Color::operator +(const Color &c) const
{
   Color result;

   result.r = this->r + c.r;
   result.g = this->g + c.g;
   result.b = this->b + c.b;
   result.a = this->a + c.a;

   return result;
}

Color Color::operator +(float s) const
{
   Color result;

   result.r = this->r + s;
   result.g = this->g + s;
   result.b = this->b + s;
   result.a = this->a + s;

   return result;
}

Color Color::operator -(const Color &c) const
{
   Color result;

   result.r = this->r - c.r;
   result.g = this->g - c.g;
   result.b = this->b - c.b;
   result.a = this->a - c.a;

   return result;
}

Color Color::operator -(float s) const
{
   Color result;

   result.r = this->r - s;
   result.g = this->g - s;
   result.b = this->b - s;
   result.a = this->a - s;

   return result;
}

Color Color::operator -() const
{
    Color result;
	
	result.r = -this->r;
	result.g = -this->g;
	result.b = -this->b;
    result.a = -this->a;
	
	return result;
}

Color Color::operator *(float s) const
{
   Color result;

   result.r = this->r * s;
   result.g = this->g * s;
   result.b = this->b * s;
   result.a = this->a * s;

   return result;
}

Color Color::operator *(const Color &c) const
{
   Color result;

   result.r = this->r * c.r;
   result.g = this->g * c.g;
   result.b = this->b * c.b;
   result.a = this->a * c.a;

   return result;
}

Color Color::operator /(float s) const
{
   Color result;

   result.r = this->r / s;
   result.g = this->g / s;
   result.b = this->b / s;
   result.a = this->a / s;

   return result;
}

Color Color::operator /(const Color &c) const
{
   Color result;

   result.r = this->r / c.r;
   result.g = this->g / c.g;
   result.b = this->b / c.b;
   result.a = this->a / c.a;

   return result;
}

bool Color::operator == (const Color &c)
{
    return(this->r == c.r && this->g == c.g && this->b == c.b && this->a == c.a);
}

bool Color::operator != (const Color &c)
{
    return(this->r != c.r || this->g != c.g || this->b != c.b || this->a != c.a);
}

bool Color::operator <= (const Color &c)
{
    return(this->r <= c.r && this->g <= c.g && this->b <= c.b && this->a <= c.a);
}

bool Color::operator < (const Color &c)
{
    return(this->r < c.r && this->g < c.g && this->b < c.b  && this->a < c.a);
}

bool Color::operator >= (const Color &c)
{
    return(this->r >= c.r && this->g >= c.g && this->b >= c.b && this->a >= c.a);
}

bool Color::operator > (const Color &c)
{
    return(this->r > c.r && this->g > c.g && this->b > c.b && this->a > c.a);
}


void RGB2HSL(float rgbR, float rgbG, float rgbB, float &hslH, float &hslS, float &hslL)
{
    float r = rgbR;
    float g = rgbG;
    float b = rgbB;

    float h = 0.0f;
    float s = 0.0f;
    float l = 0.0f;

    float themin, themax, delta;                                                                               
    //float3 c2= float3(0.0,0.0,0.0);                                                                         
                                                                                                           
    themin = g_min<float>(r, g_min<float>(g, b));                                                                 
    themax = g_max<float>(r, g_max<float>(g, b));                                                                 
    delta = themax - themin;                                                                                

    l = (themin + themax) * 0.5;                                                                           
    s = 0.0;                                                                                             
                                                                                                             
    if (l > 0.0 && l < 1.0)                                                                           
        s = delta / (l < 0.5 ? (2*l) : (2.0-2.0*l));                                                
                                                                                                           
    h = 0.0;                                                                                             
                                                                                                           
    if (delta > 0.0)                                                                                        
    {                                                                                                          
        if (themax == r && themax != g)                                                              
            h += (g - b) / delta;                                                                  
        if (themax == g && themax != b)                                                              
            h += (2.0 + (b - r) / delta);                                                          
        if (themax == b && themax != r)                                                              
            h += (4.0 + (r - g) / delta);                                                          
                                                                                                          
        h *= 60.0;                                                                                        
        h = h / 360.0;                                                                                 
    }      

    hslH = h;
    hslS = s;
    hslL = l;
}

