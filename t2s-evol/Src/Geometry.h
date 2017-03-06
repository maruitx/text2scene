#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "Math.h"
#include <vector>
#include "Utility.h"
#include <algorithm>



#define rz_min2(a, b) std::min(a, b)
#define rz_max2(a, b) std::max(a, b)        
#define rz_min4(a, b, c, d) (rz_min2(a, rz_min2(b, rz_min2(c, d))))
#define rz_max4(a, b, c, d) (rz_max2(a, rz_max2(b, rz_max2(c, d))))

class Camera;
struct Transform;

const float ZERO_EPSILON = 1e-6f;
const float RAY_TMIN = 1e-3f;
const float RAY_TMAX = 1e6f;

class Ray {
public:
	Ray() : o(make_float3(0)), d(make_float3(0)), inv_d(make_float3(0)), org_inv_d(make_float3(0)) {}
	Ray(const float3 &_o, const float3 &_d) : o(_o), d(normalize(_d)) {
		// avoid NaN as this will cause traversal performance dropped a few hundred times 
		if (isnan3(d))
			d = make_float3(0.0f, 0.0f, 0.0f);

		// avoid NaN here otherwise box hit checking is not accurate when one of elements in d is zero.                
		float3 dd = make_float3(
			fabs(d.x) < ZERO_EPSILON ? ZERO_EPSILON : d.x,
			fabs(d.y) < ZERO_EPSILON ? ZERO_EPSILON : d.y,
			fabs(d.z) < ZERO_EPSILON ? ZERO_EPSILON : d.z);

		inv_d = make_float3(1.0f) / dd;
		org_inv_d = -o * inv_d;
	}
	Ray(const Ray &r) { *this = r; }

	float3 org() const { return o; }
	float3 dir() const { return d; }
	float3 point(float t) const { return o + t * d; }

public:
	float3 o, d, inv_d, org_inv_d;
};

class BoundingBox
{
    public:
		BoundingBox() : m_mi(vec3()), m_ma(vec3()) { v_min = make_float3(0); v_max = make_float3(0); }
		BoundingBox(const vec3 &mi, const vec3 &ma) : m_mi(mi), m_ma(ma) { v_min = make_float3(m_mi); v_max = make_float3(m_ma); }
		BoundingBox(const BoundingBox &bb) : m_mi(bb.mi()), m_ma(bb.ma()) { v_min = make_float3(m_mi); v_max = make_float3(m_ma); }

		BoundingBox(const float3 &p0, const float3 &p1, const float3 &p2) {
			//v_min = min(min(p0, p1), p2);
			//v_max = max(max(p0, p1), p2);

			m_mi = vec3(v_min.x, v_min.y, v_min.z);
			m_ma = vec3(v_max.x, v_max.y, v_max.z);
		}

        ~BoundingBox() {}

        BoundingBox &operator = (const BoundingBox &bb) { m_mi = bb.mi(); m_ma = bb.ma();
		v_min = make_float3(m_mi); v_max = make_float3(m_ma);
		return *this; }

        vec3 mi() const { return m_mi; } 
        vec3 ma() const { return m_ma; }
        vec3 center() const { return ((m_mi + m_ma) * 0.5); }

		void setMin(const vec3 &mi) { m_mi = mi; v_min = make_float3(m_mi); }
		void setMax(const vec3 &ma) { m_ma = ma; v_max = make_float3(m_ma); }
		void setMinMax(const vec3 &mi, const vec3 &ma) { m_mi = mi; m_ma = ma; v_min = make_float3(m_mi); v_max = make_float3(m_ma); }

		/**
		* Return true if two boxes have intersection.
		*/
		bool overlap(const BoundingBox &b) const {
			bool x = (m_ma.x >= b.m_mi.x) && (m_mi.x <= b.m_ma.x);
			bool y = (m_ma.y >= b.m_mi.y) && (m_mi.y <= b.m_ma.y);
			bool z = (m_ma.z >= b.m_mi.z) && (m_mi.z <= b.m_ma.z);
			return (x && y && z);
		}

		void merge(const BoundingBox &b) {
			//v_min = min(v_min, b.v_min);
			//v_max = max(v_max, b.v_max);

			m_mi = vec3(v_min.x, v_min.y, v_min.z);
			m_ma = vec3(v_max.x, v_max.y, v_max.z);
		}

		bool inside(const vec3 &p, double delta = 0) const
		{
			bool xIn = (p.x >= m_mi.x + delta && p.x <= m_ma.x - delta);
			bool yIn = (p.y >= m_mi.y + delta && p.y <= m_ma.y - delta);
			bool zIn = (p.z >= m_mi.z + delta&& p.z <= m_ma.z - delta);

			return xIn&&yIn&&zIn;
		}

		// a AABB from input AABB after transformation
		BoundingBox transformBoudingBox(const mat4 &transMat) const
		{
			// get 8 vertices of the input bb
			std::vector<vec3> vertices(8);
			vertices[0] = vec3(m_mi.x, m_mi.y, m_mi.z);
			vertices[1] = vec3(m_ma.x, m_mi.y, m_mi.z);
			vertices[2] = vec3(m_ma.x, m_ma.y, m_mi.z);
			vertices[3] = vec3(m_mi.x, m_ma.y, m_mi.z);

			vertices[4] = vec3(m_mi.x, m_mi.y, m_ma.z);
			vertices[5] = vec3(m_ma.x, m_mi.y, m_ma.z);
			vertices[6] = vec3(m_ma.x, m_ma.y, m_ma.z);
			vertices[7] = vec3(m_mi.x, m_ma.y, m_ma.z);

			vec3 mi = vec3(math_maxfloat);
			vec3 ma = vec3(math_minfloat);

			// transform all vertices and find max, min
			for (int i = 0; i < 8; i++)
			{
				vec3 transVert = TransformPoint(transMat, vertices[i]);

				if (transVert.x < mi.x)
					mi.x = transVert.x;
				if (transVert.y < mi.y)
					mi.y = transVert.y;
				if (transVert.z < mi.z)
					mi.z = transVert.z;

				if (transVert.x > ma.x)
					ma.x = transVert.x;
				if (transVert.y > ma.y)
					ma.y = transVert.y;
				if (transVert.z > ma.z)
					ma.z = transVert.z;
			}

			return BoundingBox(mi, ma);
		}

		bool hit(const Ray &r, float tmin, float tmax) const {
			float t;
			return hit(r, tmin, tmax, t);
		}

		bool hit(const Ray &r, float tmin, float tmax, float &t_hit) const {
			// from Timo Aila's HPG'12 Understanding the efficiency of ray traversal on the GPUs
			float3 v0 = v_min * r.inv_d + r.org_inv_d;
			float3 v1 = v_max * r.inv_d + r.org_inv_d;
			float x0 = v0.x, y0 = v0.y, z0 = v0.z,
				x1 = v1.x, y1 = v1.y, z1 = v1.z;

			float tminbox = rz_max4(tmin, rz_min2(x0, x1), rz_min2(y0, y1), rz_min2(z0, z1));
			float tmaxbox = rz_min4(tmax, rz_max2(x0, x1), rz_max2(y0, y1), rz_max2(z0, z1));

			t_hit = tminbox;
			return (tminbox <= tmaxbox) &&
				(tmax >= tminbox && tmaxbox >= tmin); /* some overlap of [tmin, tmax] and [tminbox, tmaxbox] */
		}

public:
	float3 v_min, v_max;

    private:
        vec3 m_mi;
        vec3 m_ma;
};

class Vertex3P
{
    public:
        Vertex3P() : position(vec3()) {}
        Vertex3P(const vec3 &pos) : position(pos){}
        Vertex3P(const Vertex3P &v) : position(v.position) {}       

        ~Vertex3P() {}

        Vertex3P &operator = (const Vertex3P &v) { position = v.position; return *this;}
        void set(const vec3 &pos) { position = pos;}

        vec3 position;
};

class Vertex3P4C
{
    public:
        Vertex3P4C() : position(vec3()), color(vec4()) {}
        Vertex3P4C(const vec3 &pos, const vec4 &col) : position(pos), color(col) {}
        Vertex3P4C(const Vertex3P4C &v) : position(v.position), color(v.color) {}       

        ~Vertex3P4C() {}

        Vertex3P4C &operator = (const Vertex3P4C &v) { position = v.position; color = v.color; return *this;}
        void set(const vec3 &pos, const vec4 &col) { position = pos; color = col;}

        vec3 position;
        vec4 color;
};

class Vertex3P3N4C2T
{
    public:
        Vertex3P3N4C2T() : position(vec3()), normal(vec3()),  color(vec4()), texture(vec2()) {}
        Vertex3P3N4C2T(const vec3 &pos, const vec3 &nor, const vec4 &col, const vec2 &tex) : position(pos), normal(nor), color(col), texture(tex) {}
        Vertex3P3N4C2T(const Vertex3P3N4C2T &v) : position(v.position), normal(v.normal), color(v.color), texture(v.texture) {}       

        ~Vertex3P3N4C2T() {}

        Vertex3P3N4C2T &operator = (const Vertex3P3N4C2T &v) { position = v.position; normal = v.normal; color = v.color; texture = v.texture; return *this;}
        void set(const vec3 &pos, const vec3 &nor, const vec4 &col, const vec2 &tex) { position = pos; normal = nor; color = col; texture = tex;}

        vec3 position;
        vec3 normal;
        vec4 color;
        vec2 texture;
};

class Vertex4P4N4C4T
{
    public:
        Vertex4P4N4C4T() : position(vec4()), normal(vec4()),  color(vec4()), texture(vec4()) {}
        Vertex4P4N4C4T(const vec4 &pos, const vec4 &nor, const vec4 &col, const vec4 &tex) : position(pos), normal(nor), color(col), texture(tex) {}
        Vertex4P4N4C4T(const Vertex4P4N4C4T &v) : position(v.position), normal(v.normal), color(v.color), texture(v.texture) {}       

        ~Vertex4P4N4C4T() {}

        Vertex4P4N4C4T &operator = (const Vertex4P4N4C4T &v) { position = v.position; normal = v.normal; color = v.color; texture = v.texture; return *this;}
        void set(const vec4 &pos, const vec4 &nor, const vec4 &col, const vec4 &tex) { position = pos; normal = nor; color = col; texture = tex;}

        vec4 position;
        vec4 normal;
        vec4 color;
        vec4 texture;
};

typedef Vertex3P3N4C2T Vertex;

class Face 
{
public: 
    Face(uint _a, uint _b, uint _c) : a(_a), b(_b), c(_c) {}
    Face(const Face &f) : a(f.a), b(f.b), c(f.c) {}    
    Face &operator = (const Face &f) { a = f.a; b = f.b; c = f.c; }

    uint a, b, c;
};

class Triangle
{
public:       
   
    Triangle();
    Triangle(const vec3 &va, const vec3 &vb, const vec3 &vc, const vec3 &vn, int ifa, int ifb, int ifc);
    Triangle(const vec3 &va, const vec3 &vb, const vec3 &vc, const vec3 &vn, const vec3 &vna, const vec3 &vnb, const vec3 &vnc, int ifa, int ifb, int ifc);
    Triangle(const Triangle &t);
    ~Triangle();

    Triangle &operator =  (const Triangle &t);
    bool intersect(const vec3 &rayStart, const vec3 &rayDir, float &t) const;
	bool intersect(vec3 rayStart, vec3 rayDir, vec3 a, vec3 b, vec3 c, float &t) const;
	bool intersect(const vec3 &rayStart, const vec3 &rayDir, vec3 &point) const;
	bool intersect(vec3 rayStart, vec3 rayDir, vec3 v0, vec3 v1, vec3 v2, vec3 &point) const;
    bool inside(const vec3 &p) const;
	
	vec3 getBarycentric(const vec3 &p);
	double getArea();

	/*
	Get neighbour triangle index depending on barycentric coordinates
	Returns -1 if no neighbour found
	!ASSUMES at least one coordinate is negative
	*/
	int getNeighbourCrossing(const vec3 &pbary, vec3 * v0_out = nullptr, vec3 * v1_out = nullptr);
	int getNeighbourCrossingIntersection(const vec3& origin, const vec3& dir, float * t, vec3 * edgeNormal, float maxDist);
    int shareCommonEdge(const Triangle &t, vec3 &s, vec3 &e);

    class Neighbor
    {
    public:

        Neighbor(const vec3 &_s, const vec3 _t, uint _idx) : s(_s), t(_t), idx(_idx) {};

        vec3 s;
        vec3 t;

        int idx;
    };

public:
    int fa;
    int fb;
    int fc;

    vec3 a;
    vec3 b;
    vec3 c;

    vec3 n;

    vec3 center;

    std::vector<Neighbor> neighbors;

    bool perVertexNormal;

    vec3 na;
    vec3 nb;
    vec3 nc;

};

class Picking
{
private:
    vec3 pickingRay(Camera* camera, int mouse_x, int mouse_y, float window_width, float window_height);
    void getPickingRay(const Transform &trans, const float fov, const float ncp, const float window_width, const float window_height, float mouse_x, float mouse_y, vec3 &rayPos, vec3 &rayDir);
    vec3 intersectionPlane(vec3 v1, vec3 v2, vec3 v3, vec3 startPos, vec3 endPos);
    bool intersectTriangle(vec3 rayStart, vec3 rayDir, vec3 a, vec3 b, vec3 c, float &t);
    bool intersectQuad(vec3 rayStart, vec3 rayDir, vec3 v0, vec3 v1, vec3 v2, vec3 v3, float &t);
    bool intersectCube(vec3 rayStart, vec3 rayDir, vec3 v0, vec3 v1, vec3 v2, vec3 v3, vec3 v4, vec3 v5, vec3 v6, vec3 v7, float &t);    

public:
    Picking();
    ~Picking();

    float select(const Transform &trans, const mat4 &matModel, const vec3 &mi, const vec3 &ma, int width, int height, int mx, int my);
};

class Plane  
{
public:
	vec3 normal, point;
	float d;

	Plane(vec3 &v1,  vec3 &v2,  vec3 &v3);
	Plane();
	~Plane();

	void set3Points(vec3 &v1,  vec3 &v2,  vec3 &v3);
	void setNormalAndPoint(vec3 &normal, vec3 &point);
	void setCoefficients(float a, float b, float c, float d);
	float distance(vec3 p);

	void print();
};

class Frustum 
{
private:

	enum {
		TOP = 0,
		BOTTOM,
		LEFT,
		RIGHT,
		NEARP,
		FARP
	};

    float m_angle;

public:
	static enum {OUTSIDE, INTERSECT, INSIDE};

	Plane pl[6];

	vec3 ntl,ntr,nbl,nbr,ftl,ftr,fbl,fbr;
	float nearD, farD, ratio, angle,tang;
	float nw,nh,fw,fh;

	Frustum();
	~Frustum();

	void setCamInternals(float angle, float ratio, float nearD, float farD);
	void setCamDef(vec3 &p, vec3 &l, vec3 &u);
	int pointInFrustum(vec3 &p);
	int sphereInFrustum(vec3 &p, float raio);
	//int boxInFrustum(AABox &b);
    int boxInFrustum(vec3 &min, vec3 &max);

    vec3 getVertexN(vec3 &normal, vec3 &min, vec3 &max);
    vec3 getVertexP(vec3 &normal, vec3 &min, vec3 &max);

	void drawPoints();
	void drawLines();
	void drawPlanes();
	void drawNormals();
};

class Spline
{
public:
    enum Config
    {
        CATMULL_ROM = 0,
        CUBIC, 
        HERMITE,
        PICEWISE_HERMITE,
        COSINE,
        LINEAR,
        KOCHANEK_BARTEL,
        ROUNDED_CATMULL_ROM,
        BSPLINE
    };
    
    Spline(Config conf = CATMULL_ROM);
   ~Spline();

   void addPoint(const vec3 &v);
   void clear();
   vec3 interpolatedPoint(float t, Config conf = CATMULL_ROM);
   vec3 point(int n) const;
   int numPoints() const;
   void render(Config conf = CATMULL_ROM);

   void bounds(int &p);

   static vec3 linearInterpolation(const vec3 &p0, const vec3 &p1, float t);
   static vec3 catmullRomInterpolation(const vec3 &p0, const vec3 &p1, const vec3 &p2, const vec3 &p3, float t);
   static vec3 roundedCatmullRomInterpolation(const vec3 &p0, const vec3 &p1, const vec3 &p2, const vec3 &p3, float t);
   static vec3 cubicInterpolation(const vec3 &p0, const vec3 &p1, const vec3 &p2, const vec3 &p3, double t);
   static vec3 bSplineInterpolation(const vec3 &p1, const vec3 &p2, const vec3 &p3, const vec3 &p4, double t);
   static vec3 hermiteInterpolation(const vec3 &p0, const vec3 &p1, const vec3 &p2, const vec3 &p3, double t, double tension = 0.0, double bias = 0.0);
   static vec3 picewiseHermiteInterpolation(const vec3 &a, const vec3 &b, const vec3 &startTangent, const vec3 &endTangent, float t);
   static vec3 kochanekBartelInterpolation(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d, double t, double tension = 0.0, double bias = 0.0, double continuity = 0.0f);
   
   std::vector<vec3> m_points;
   vec3 m_phantomStart;
   vec3 m_phantomEnd;

private:    
    float m_deltaT;
    Config m_config;
};

int shareCommonEdge(Face &f1, Face &f2);
float rayPlaneIntersection(const vec3 &origin, const vec3 &dir, const vec3& planeN, const vec3& planePt);
vec3 projectVectorOnPlane(const vec3 &u, const vec3 &n);
float pointLineDistance(const vec3 &a, const vec3 &b, const vec3 &p);
bool isInsideMesh(const std::vector<Triangle> &triangles, const vec3 &point);
void normalizeGeometry(std::vector<Vertex> &vertices, const vec3 &translate, const vec3 &scale, const vec4 &rotate);

#endif