// Constants
#define PI 3.14159265359
#define SUB_SAMPLES 1
#define EPSILON 1e-4
#define RAY_EPSILON 1e-3
#define MAX_DEPTH 64

// Material Types (���������, ���淴�����, �������)
#define DIFF 0
#define SPEC 1
#define REFR 2

struct Ray {
	vec3 origin;	// ����ԭ��
	vec3 dir; 		// ���߷���
};

struct Material {
	int refl;	    // ��������(DIFF, SPEC, REFR)
	vec3 emission;	// �Է���
	vec3 color;		// ��ɫ
	float ior;		// ������
};

struct Sphere {
	float radius;	// �뾶
	vec3 pos;		// λ��
	Material mat;	// ����
};

struct Plane {
	vec3 pos;		// λ��
	vec3 normal;	// ����
	Material mat;	// ����
};

// Util functions
float seed = 0.;
float rand() { return fract(sin(seed++)*43758.5453123); }

vec3 cosWeightedSampleHemisphere(vec3 n) {
	float u1 = rand(), u2 = rand();
	float r = sqrt(u1);
	float theta = 2. * PI * u2;

	float x = r * cos(theta);
	float y = r * sin(theta);
	float z = sqrt(max(0., 1. - u1));

	vec3 a = n, b;

	if (abs(a.x) <= abs(a.y) && abs(a.x) <= abs(a.z))
		a.x = 1.0;
	else if (abs(a.y) <= abs(a.x) && abs(a.y) <= abs(a.z))
		a.y = 1.0;
	else
		a.z = 1.0;

	a = normalize(cross(n, a));
	b = normalize(cross(n, a));

	return normalize(a * x + b * y + n * z);
}

// Scene Description
#define NUM_SPHERES 3
#define NUM_PLANES 6
Sphere spheres[NUM_SPHERES];
Plane planes[NUM_PLANES];

void initScene() {
	spheres[0] = Sphere(16.5, vec3(27, 16.5, 47), Material(SPEC, vec3(0.), vec3(1.), 0.));
	spheres[1] = Sphere(16.5, vec3(73, 16.5, 78), Material(REFR, vec3(0.), vec3(.75, 1., .75), 1.5));
	spheres[2] = Sphere(600., vec3(50, 689.3, 50), Material(DIFF, vec3(6.), vec3(0.), 0.));

	planes[0] = Plane(vec3(0, 0, 0), vec3(0, 1, 0), Material(DIFF, vec3(0.), vec3(.75), 0.));
	planes[1] = Plane(vec3(-7, 0, 0), vec3(1, 0, 0), Material(DIFF, vec3(0.), vec3(.75, .25, .25), 0.));
	planes[2] = Plane(vec3(0, 0, 0), vec3(0, 0, -1), Material(DIFF, vec3(0.), vec3(.75), 0.));
	planes[3] = Plane(vec3(107, 0, 0), vec3(-1, 0, 0), Material(DIFF, vec3(0.), vec3(.25, .25, .75), 0.));
	planes[4] = Plane(vec3(0, 0, 180), vec3(0, 0, 1), Material(DIFF, vec3(0.), vec3(0.), 0.));
	planes[5] = Plane(vec3(0, 90, 0), vec3(0, -1, 0), Material(DIFF, vec3(0.), vec3(.75), 0.));
}

vec3 background(vec3 dir) {
	//return mix(vec3(0.), vec3(.9), .5 + .5 * dot(dir, vec3(0., 1., 0.)));
	return texture(iChannel1, dir).rgb;
}

// ��������Բ�ཻ
float intersectSphere(Ray r, Sphere s) {
	vec3 op = s.pos - r.origin;
	float b = dot(op, r.dir);

	float delta = b * b - dot(op, op) + s.radius * s.radius;
	if (delta < 0.)           // ����������δ�ཻ
		return 0.;
	else                      // �����������ཻ
		delta = sqrt(delta);

	float t;                  // �ҵ�t��С�Ľ���
	if ((t = b - delta) > EPSILON)
		return t;
	else if ((t = b + delta) > EPSILON)
		return t;
	else
		return 0.;
}

float intersectPlane(Ray r, Plane p) {
	float t = dot(p.pos - r.origin, p.normal) / dot(r.dir, p.normal);
	return mix(0., t, float(t > EPSILON));
}

// ���������������ཻ���ҵ��ཻ�ļ����壬�����ཻ�������ID
int intersect(Ray ray, out float t, out vec3 normal, out Material mat) {
	int id = -1;
	t = 1e5;
	for (int i = 0; i < NUM_SPHERES; i++) {
		float d = intersectSphere(ray, spheres[i]);
		if (d != 0. && d<t) {
			id = i;
			t = d;
			normal = normalize(ray.origin + ray.dir * t - spheres[i].pos);
			mat = spheres[i].mat;
		}
	}

	for (int i = 0; i < NUM_PLANES; i++) {
		float d = intersectPlane(ray, planes[i]);
		if (d != 0. && d < t) {
			id = i;
			t = d;
			normal = planes[i].normal;
			mat = planes[i].mat;
		}
	}
	return id;
}

// �������ģ�����ɳ�ʼ����
Ray generateRay(vec2 uv) {
	vec2 p = uv * 2. - 1.;

	vec3 camPos = vec3(50., 40.8, 172.);
	vec3 cz = normalize(vec3(50., 40., 81.6) - camPos);
	vec3 cx = vec3(1., 0., 0.);
	vec3 cy = normalize(cross(cx, cz));
	cx = cross(cz, cy);

	float aspectRatio = iResolution.x / iResolution.y;
	return Ray(camPos, normalize(.5135 * (aspectRatio * p.x * cx + p.y * cy) + cz));
}

// ����ȼ���
vec3 trace(Ray ray) {
	vec3 radiance = vec3(0.);		 // �ۻ������
	vec3 reflectance = vec3(1.);	 // �ۻ�������
	for (int depth = 0; depth < MAX_DEPTH; depth++) {
		float t;	    // �ཻ������
		vec3 n;			// �ཻ��ķ���
		Material mat;   // �ཻ������

		int id = intersect(ray, t, n, mat);

		// ���û�к������ཻ�����ر����ķ����
		if (id < 0) {
			radiance += reflectance * background(ray.dir);
			break;
		}


		// �ۼ���һ�εķ����
		radiance += reflectance * mat.emission;

		vec3 color = mat.color;
		// ������������
		float p = max(color.x, max(color.y, color.z));
		// Russain roulette
		if (rand() < p)
			color /= p;
		else
			break;

		// ���ݹ����뷨�ߵķ����ж������뻹�����������ת����
		vec3 nl = n * sign(-dot(n, ray.dir));
		// �����ߵ�ԭ���ƶ����ཻ��
		ray.origin += ray.dir * t;


		if (mat.refl == DIFF) {				// ���������
			ray.dir = cosWeightedSampleHemisphere(nl);
			reflectance *= color;
		}
		else if (mat.refl == SPEC) {	    // ���淴�����
			ray.dir = reflect(ray.dir, n);
			reflectance *= color;
		}
		else {						    // �������
			float ior = mat.ior;
			float into = float(dot(n, nl) > 0.);	 // �����Ƿ���ⲿ����
			float ddn = dot(nl, ray.dir);
			float nnt = mix(ior, 1. / ior, into);
			vec3 rdir = reflect(ray.dir, n);
			float cos2t = 1. - nnt * nnt * (1. - ddn * ddn);
			if (cos2t > 0.) {		// �Ƿ���Total Internal Reflection
									// ���������ߵķ���
				vec3 tdir = normalize(ray.dir * nnt - nl * (ddn * nnt + sqrt(cos2t)));

				float R0 = (ior - 1.) * (ior - 1.) / ((ior + 1.) * (ior + 1.));
				float c = 1. - mix(dot(tdir, n), -ddn, into);	// 1 - cos��
				float Re = R0 + (1. - R0) * c * c * c * c * c;	// ��������

				float P = .25 + .5 * Re;			  // �������
				float RP = Re / P, TP = (1. - Re) / (1. - P);   // ����/����ϵ��

																// Russain roulette
				if (rand() < P) {				// ѡ����
					reflectance *= RP;
					ray.dir = rdir;
				}
				else { 				        // ѡ������
					reflectance *= color * TP;
					ray.dir = tdir;
				}
			}
			else
				ray.dir = rdir;
		}

		// ��������ǰ�ƽ�һ�㣬��ֹ���ཻ
		ray.origin += ray.dir * RAY_EPSILON;
	}
	return radiance;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
	// ��ʼ�����������
	seed = iTime + iResolution.y * fragCoord.x / iResolution.x + fragCoord.y / iResolution.y;

	// ��ʼ������
	initScene();

	vec3 color = vec3(0.);
	// ������
	for (int x = 0; x < SUB_SAMPLES; x++) {
		for (int y = 0; y < SUB_SAMPLES; y++) {
			// Tent Filter
			float r1 = 2. * rand(), r2 = 2. * rand();
			float dx = mix(sqrt(r1) - 1., 1. - sqrt(2. - r1), float(r1 > 1.));
			float dy = mix(sqrt(r2) - 1., 1. - sqrt(2. - r2), float(r2 > 1.));
			vec2 jitter = vec2(dx, dy);

			// ���������ڲ������uv����
			vec2 subuv = (fragCoord.xy + jitter) / iResolution.xy;

			// �����������
			Ray camRay = generateRay(subuv);

			// ������߶�Ӧ�ķ����
			color += trace(camRay);
		}
	}

	color /= float(SUB_SAMPLES * SUB_SAMPLES);

	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord.xy / iResolution.xy;

	// muiltpass ��β������ƽ�����
	color += texture(iChannel0, uv).rgb * float(iFrame);
	fragColor = vec4(color / float(iFrame + 1), 1.);
}

#define GAMMA 2.2f

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
	vec2 uv = fragCoord.xy / iResolution.xy;
	vec3 color = texture(iChannel0, uv).rgb;

	fragColor = vec4(clamp(pow(color, vec3(1. / GAMMA)), 0., 1.), 1.);
}