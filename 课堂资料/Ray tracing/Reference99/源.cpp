#include <math.h>   // smallpt, a Path Tracer by Kevin Beason, 2008 
#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt 
#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2 
#include <random>

#define M_PI 3.14159265358979323846
const double infinity = 1e20;

//随机数封装
std::default_random_engine generator;
std::uniform_real_distribution<double> distr(0.0, 1.0);
double erand48(unsigned short * X) {
	return distr(generator);
}

//Vector3结构
struct Vec {        // Usage: time ./smallpt 5000 && xv image.ppm 
	double x, y, z;                  // position, also color (r,g,b) 
	Vec(double x = 0, double y = 0, double z = 0) :x(x), y(y), z(z) {  }
	Vec operator+(const Vec &b) const {
		return Vec(x + b.x, y + b.y, z + b.z);
	}
	Vec operator-(const Vec &b) const {
		return Vec(x - b.x, y - b.y, z - b.z);
	}
	Vec operator*(double b) const {
		return Vec(x*b, y*b, z*b);
	}
	double operator*(const Vec &b) const {
		return x*b.x + y*b.y + z*b.z;
	}
	Vec mult(const Vec &b) const {
		return Vec(x*b.x, y*b.y, z*b.z);
	}
	Vec& norm() {
		return *this = *this * (1 / sqrt(x*x + y*y + z*z));
	}
	double dot(const Vec &b) const {
		return x*b.x + y*b.y + z*b.z;
	} // cross: 
	Vec operator%(Vec&b) {
		return Vec(y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x);
	}
};

//Ray结构
struct Ray {
	Vec origin, direction;
	Ray(Vec origin, Vec direction) : origin(origin), direction(direction) {}
};

//反射类型
enum ReflectType { DIFF, SPEC, REFR };  // material types, used in radiance() 

struct Sphere {
	double rad;       // radius 
	Vec position, emission, color;      // position, emission, color 
	ReflectType refl;      // reflection type (DIFFuse, SPECular, REFRactive) 
	Sphere(double rad, Vec position, Vec emission, Vec color, ReflectType refl) :
		rad(rad), position(position), emission(emission), color(color), refl(refl) {}
	double intersect(const Ray &ray) const { // returns distance, 0 if nohit 
		Vec op = position - ray.origin; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0 
		double eps = 1e-4;
		double b = op.dot(ray.direction);
		double  det = b*b - op*op + rad*rad;

		if (det < 0) {
			return 0;
		}
		else
		{
			det = sqrt(det);
		}

		double t;
		return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
	}
};
Sphere spheres[] = {//Scene: radius, position, emission, color, material 
	Sphere(1e5, Vec(1e5 + 1,40.8,81.6), Vec(),Vec(.75,.25,.25),DIFF),//Left 
	Sphere(1e5, Vec(-1e5 + 99,40.8,81.6),Vec(),Vec(.25,.25,.75),DIFF),//Rght 
	Sphere(1e5, Vec(50,40.8, 1e5),     Vec(),Vec(.75,.75,.75),DIFF),//Back 
	Sphere(1e5, Vec(50,40.8,-1e5 + 170), Vec(),Vec(),           DIFF),//Frnt 
	Sphere(1e5, Vec(50, 1e5, 81.6),    Vec(),Vec(.75,.75,.75),DIFF),//Botm 
	Sphere(1e5, Vec(50,-1e5 + 81.6,81.6),Vec(),Vec(.75,.75,.75),DIFF),//Top 
	Sphere(16.5,Vec(27,16.5,47),       Vec(),Vec(1,1,1)*.999, SPEC),//Mirr 
	Sphere(16.5,Vec(73,16.5,78),       Vec(),Vec(1,1,1)*.999, REFR),//Glas 
	Sphere(600, Vec(50,681.6 - .27,81.6),Vec(12,12,12),  Vec(), DIFF) //Lite 
};

inline double clamp(double x) { return x < 0 ? 0 : x>1 ? 1 : x; }
inline int toInt(double x) { return int(pow(clamp(x), 1 / 2.2) * 255 + .5); }

inline bool intersect(const Ray &ray, double &distance, int &id) {
	double n = sizeof(spheres) / sizeof(Sphere);
	double direction;
	distance = infinity;	//无限常量infinity
	for (int i = int(n); i--;) {
		//找到最近的反射点
		if ((direction = spheres[i].intersect(ray)) && direction < distance) {
			distance = direction; id = i;
		}
	}
	return distance < infinity;
}

Vec radiance(const Ray &ray, int depth, unsigned short *Xi) {
	double distance;                               // distance to intersection 
	int id = 0;                               // id of intersected object 
	if (!intersect(ray, distance, id)) return Vec(); // if miss, return black 
	const Sphere &obj = spheres[id];        // the hit object 

	Vec x = ray.origin + ray.direction*distance;
	Vec n = (x - obj.position).norm();
	Vec nl = n*ray.direction < 0 ? n : n*-1;
	Vec f = obj.color;

	double maxRef = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z; // max refl 

	depth++;
	if (depth > 5) {
		if (erand48(Xi) < maxRef)
			f = f*(1 / maxRef);
		else
			return obj.emission; //R.R.
	}

	if (obj.refl == DIFF) {                  // Ideal DIFFUSE reflection 
		double r1 = 2 * M_PI *erand48(Xi);
		double r2 = erand48(Xi);
		double r2s = sqrt(r2);
		Vec w = nl;
		Vec u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)) % w).norm();
		Vec v = w%u;
		Vec direction = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1 - r2)).norm();
		return obj.emission + f.mult(radiance(Ray(x, direction), depth, Xi));
	}
	else if (obj.refl == SPEC) {            // Ideal SPECULAR reflection 
		return obj.emission + f.mult(radiance(Ray(x, ray.direction - n * 2 * n.dot(ray.direction)), depth, Xi));
	}
	Ray reflRay(x, ray.direction - n * 2 * n.dot(ray.direction));     // Ideal dielectric REFRACTION 
	bool into = n.dot(nl) > 0;                // Ray from outside going in? 
	double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = ray.direction.dot(nl), cos2t;
	if ((cos2t = 1 - nnt*nnt*(1 - ddn*ddn)) < 0)    // Total internal reflection 
		return obj.emission + f.mult(radiance(reflRay, depth, Xi));
	Vec tdir = (ray.direction*nnt - n*((into ? 1 : -1)*(ddn*nnt + sqrt(cos2t)))).norm();
	double a = nt - nc, b = nt + nc, R0 = a*a / (b*b), color = 1 - (into ? -ddn : tdir.dot(n));
	double Re = R0 + (1 - R0)*color*color*color*color*color, Tr = 1 - Re, P = .25 + .5*Re, RP = Re / P, TP = Tr / (1 - P);
	return obj.emission + f.mult(depth > 2 ? (erand48(Xi) < P ?   // Russian roulette 
		radiance(reflRay, depth, Xi)*RP : radiance(Ray(x, tdir), depth, Xi)*TP) :
		radiance(reflRay, depth, Xi)*Re + radiance(Ray(x, tdir), depth, Xi)*Tr);
}
int main(int argc, char *argv[]) {
	int w = 1024, h = 768, samps = argc == 2 ? atoi(argv[1]) / 4 : 1; // # samples 
	Ray cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()); // cam pos, dir 
	Vec cx = Vec(w*.5135 / h), cy = (cx%cam.direction).norm()*.5135, r, *color = new Vec[w*h];
#pragma omp parallel for schedule(dynamic, 1) private(ray)       // OpenMP 
	for (int y = 0; y < h; y++) {                       // Loop over image rows 
		fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * 4, 100.*y / (h - 1));
		for (unsigned short x = 0, Xi[3] = { 0,0,y*y*y }; x < w; x++)   // Loop cols 
			for (int sy = 0, i = (h - y - 1)*w + x; sy < 2; sy++)     // 2x2 subpixel rows 
				for (int sx = 0; sx < 2; sx++, r = Vec()) {        // 2x2 subpixel cols 
					for (int s = 0; s < samps; s++) {
						double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
						double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
						Vec direction = cx*(((sx + .5 + dx) / 2 + x) / w - .5) +
							cy*(((sy + .5 + dy) / 2 + y) / h - .5) + cam.direction;
						r = r + radiance(Ray(cam.origin + direction * 140, direction.norm()), 0, Xi)*(1. / samps);
					} // Camera rays are pushed ^^^^^ forward to start in interior 
					color[i] = color[i] + Vec(clamp(r.x), clamp(r.y), clamp(r.z))*.25;
				}
	}
	FILE *f;
	fopen_s(&f, "image.ppm", "w");// Write image to PPM file. 
	fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);
	for (int i = 0; i < w*h; i++)
		fprintf(f, "%d %d %d ", toInt(color[i].x), toInt(color[i].y), toInt(color[i].z));
}