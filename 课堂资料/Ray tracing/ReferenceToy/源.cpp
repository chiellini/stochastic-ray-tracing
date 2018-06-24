// Constants
#define PI 3.14159265359
#define SUB_SAMPLES 1
#define EPSILON 1e-4
#define RAY_EPSILON 1e-3
#define MAX_DEPTH 64

// Material Types (漫反射表面, 镜面反射表面, 折射表面)
#define DIFF 0
#define SPEC 1
#define REFR 2

struct Ray {
	vec3 origin;	// 光线原点
	vec3 dir; 		// 光线方向
};

struct Material {
	int refl;	    // 表面属性(DIFF, SPEC, REFR)
	vec3 emission;	// 自发光
	vec3 color;		// 颜色
	float ior;		// 折射率
};

struct Sphere {
	float radius;	// 半径
	vec3 pos;		// 位置
	Material mat;	// 材质
};

struct Plane {
	vec3 pos;		// 位置
	vec3 normal;	// 法线
	Material mat;	// 材质
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

// 检测光线与圆相交
float intersectSphere(Ray r, Sphere s) {
	vec3 op = s.pos - r.origin;
	float b = dot(op, r.dir);

	float delta = b * b - dot(op, op) + s.radius * s.radius;
	if (delta < 0.)           // 光线与球体未相交
		return 0.;
	else                      // 光线与球体相交
		delta = sqrt(delta);

	float t;                  // 找到t最小的交点
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

// 光线与整个场景相交，找到相交的几何体，返回相交几何体的ID
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

// 根据相机模型生成初始光线
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

// 辐射度计算
vec3 trace(Ray ray) {
	vec3 radiance = vec3(0.);		 // 累积辐射度
	vec3 reflectance = vec3(1.);	 // 累积反射率
	for (int depth = 0; depth < MAX_DEPTH; depth++) {
		float t;	    // 相交处距离
		vec3 n;			// 相交面的法线
		Material mat;   // 相交处材质

		int id = intersect(ray, t, n, mat);

		// 如果没有和物体相交，返回背景的辐射度
		if (id < 0) {
			radiance += reflectance * background(ray.dir);
			break;
		}


		// 累加这一次的辐射度
		radiance += reflectance * mat.emission;

		vec3 color = mat.color;
		// 反射率最大分量
		float p = max(color.x, max(color.y, color.z));
		// Russain roulette
		if (rand() < p)
			color /= p;
		else
			break;

		// 根据光线与法线的方向判断是射入还是射出，并反转法线
		vec3 nl = n * sign(-dot(n, ray.dir));
		// 将光线的原点移动到相交点
		ray.origin += ray.dir * t;


		if (mat.refl == DIFF) {				// 漫反射表面
			ray.dir = cosWeightedSampleHemisphere(nl);
			reflectance *= color;
		}
		else if (mat.refl == SPEC) {	    // 镜面反射表面
			ray.dir = reflect(ray.dir, n);
			reflectance *= color;
		}
		else {						    // 折射表面
			float ior = mat.ior;
			float into = float(dot(n, nl) > 0.);	 // 光线是否从外部进来
			float ddn = dot(nl, ray.dir);
			float nnt = mix(ior, 1. / ior, into);
			vec3 rdir = reflect(ray.dir, n);
			float cos2t = 1. - nnt * nnt * (1. - ddn * ddn);
			if (cos2t > 0.) {		// 是否发生Total Internal Reflection
									// 算出折射光线的方向
				vec3 tdir = normalize(ray.dir * nnt - nl * (ddn * nnt + sqrt(cos2t)));

				float R0 = (ior - 1.) * (ior - 1.) / ((ior + 1.) * (ior + 1.));
				float c = 1. - mix(dot(tdir, n), -ddn, into);	// 1 - cosθ
				float Re = R0 + (1. - R0) * c * c * c * c * c;	// 菲涅尔项

				float P = .25 + .5 * Re;			  // 反射概率
				float RP = Re / P, TP = (1. - Re) / (1. - P);   // 反射/折射系数

																// Russain roulette
				if (rand() < P) {				// 选择反射
					reflectance *= RP;
					ray.dir = rdir;
				}
				else { 				        // 选择折射
					reflectance *= color * TP;
					ray.dir = tdir;
				}
			}
			else
				ray.dir = rdir;
		}

		// 将光线往前推进一点，防止自相交
		ray.origin += ray.dir * RAY_EPSILON;
	}
	return radiance;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
	// 初始化随机数种子
	seed = iTime + iResolution.y * fragCoord.x / iResolution.x + fragCoord.y / iResolution.y;

	// 初始化场景
	initScene();

	vec3 color = vec3(0.);
	// 超采样
	for (int x = 0; x < SUB_SAMPLES; x++) {
		for (int y = 0; y < SUB_SAMPLES; y++) {
			// Tent Filter
			float r1 = 2. * rand(), r2 = 2. * rand();
			float dx = mix(sqrt(r1) - 1., 1. - sqrt(2. - r1), float(r1 > 1.));
			float dy = mix(sqrt(r2) - 1., 1. - sqrt(2. - r2), float(r2 > 1.));
			vec2 jitter = vec2(dx, dy);

			// 计算像素内采样点的uv坐标
			vec2 subuv = (fragCoord.xy + jitter) / iResolution.xy;

			// 生成相机光线
			Ray camRay = generateRay(subuv);

			// 计算光线对应的辐射度
			color += trace(camRay);
		}
	}

	color /= float(SUB_SAMPLES * SUB_SAMPLES);

	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord.xy / iResolution.xy;

	// muiltpass 多次采样算出平均结果
	color += texture(iChannel0, uv).rgb * float(iFrame);
	fragColor = vec4(color / float(iFrame + 1), 1.);
}

#define GAMMA 2.2f

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
	vec2 uv = fragCoord.xy / iResolution.xy;
	vec3 color = texture(iChannel0, uv).rgb;

	fragColor = vec4(clamp(pow(color, vec3(1. / GAMMA)), 0., 1.), 1.);
}