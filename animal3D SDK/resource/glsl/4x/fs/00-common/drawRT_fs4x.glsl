/*
	Copyright 2011-2026 Daniel S. Buckstein

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	animal3D SDK: Minimal 3D Animation Framework
	By Daniel S. Buckstein
	
	drawRT_fs4x.glsl
	Output ray-tracing.
*/

#version 450


////////////////////////////////////////////////////////////
/// FRAGMENT SHADER INPUTS

in vbVertexData {
	mat4 vTangentBasis_view;
	vec4 vTexcoord_atlas;
};

///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
/// FRAGMENT SHADER OUTPUTS

layout (location = 0) out vec4 rtFragColor;

///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
/// SCENE DESCRIPTORS


#define rtReal float
struct rtScalar
{
    rtReal x;
    rtReal xx;
    rtReal _x;
    rtReal _xx;
};
#define rtVector vec4
#define rtPoint  vec4
#define rtColor  vec4


// RAY-TRACING STUFF
struct rtShapePlane
{
    rtVector size_2;
};
struct rtShapeBox
{
	rtVector[2] bounds_2;
};
struct rtShapeSphere
{
    rtScalar radius;
};
struct rtMaterialDefault
{
    rtColor albedo;
    rtColor emissive;
};
struct rtObject
{
    rtPoint center;
    rtVector normal;
    rtVector tangent;
    rtVector bitangent;

    int type_shape;
    int idx_shape;
    int type_mat;
    int idx_mat;
};

#define NUM_SHAPE_TYPES                          5 
    #define IDX_SHAPE_TYPE_PLANE_INFINITE            0
        #define NUM_SHAPE_PLANE_INFINITE                 0
    #define IDX_SHAPE_TYPE_PLANE_FINITE              1   
        #define NUM_SHAPE_PLANE_FINITE                   1
            #define IDX_SHAPE_PLANE_FINITE_GROUND            0
    #define IDX_SHAPE_TYPE_BOX                       2       
        #define NUM_SHAPE_BOX                            2   
            #define IDX_SHAPE_BOX_OBJ0                       0
            #define IDX_SHAPE_BOX_OBJ1                       1
    #define IDX_SHAPE_TYPE_BOX_INVERTED              3       
        #define NUM_SHAPE_BOX_INVERTED                   1   
            #define IDX_SHAPE_BOX_INVERTED_ROOM              0
    #define IDX_SHAPE_TYPE_SPHERE                    4       
        #define NUM_SHAPE_SPHERE                         3   
            #define IDX_SHAPE_SPHERE_OBJ0                    0
            #define IDX_SHAPE_SPHERE_OBJ1                    1
            #define IDX_SHAPE_SPHERE_LIGHTBULB               2
                                                         
#define NUM_MATERIAL_TYPES                       1       
    #define IDX_MATERIAL_TYPE_DEFAULT                0   
        #define NUM_MATERIAL_DEFAULT                     8
            #define IDX_MATERIAL_DEFAULT_RED                 0
            #define IDX_MATERIAL_DEFAULT_YELLOW              1
            #define IDX_MATERIAL_DEFAULT_GREEN               2
            #define IDX_MATERIAL_DEFAULT_CYAN                3
            #define IDX_MATERIAL_DEFAULT_BLUE                4
            #define IDX_MATERIAL_DEFAULT_MAGENTA             5
            #define IDX_MATERIAL_DEFAULT_WHITE_EMIT          6
            #define IDX_MATERIAL_DEFAULT_GRAY_EMIT           7

#define UID_OBJECT_BOX0      0
#define UID_OBJECT_BOX1      1
#define UID_OBJECT_SPHERE0   2
#define UID_OBJECT_SPHERE1   3
#define UID_OBJECT_LIGHTBULB 4
#define UID_OBJECT_GROUND    5
#define UID_OBJECT_ROOM      6
#define NUM_OBJECT           7

#define NUM_OBJECT_PLANE_INFINITE 0
#define NUM_OBJECT_PLANE_FINITE   1
#define NUM_OBJECT_BOX            2
#define NUM_OBJECT_BOX_INVERTED   1
#define NUM_OBJECT_SPHERE         3

#define IDX_MODEL_BOX0			0
#define IDX_MODEL_BOX1			1
#define IDX_MODEL_SPHERE0		2
#define IDX_MODEL_SPHERE1		3
#define IDX_MODEL_LIGHT_SPHERE	4
#define IDX_MODEL_ROOM_BOX		5
#define NUM_MODEL               6

#define IDX_VIEWER_MAIN 0
#define NUM_VIEWER      1

struct rtMaterialRegistry
{
    rtMaterialDefault mat_default[NUM_MATERIAL_DEFAULT];
};
struct rtShapeRegistry
{
    //rtShapePlane  plane_infinite[NUM_SHAPE_PLANE_INFINITE];
    rtShapePlane  plane_finite[NUM_SHAPE_PLANE_FINITE];
    rtShapeBox    box[NUM_SHAPE_BOX];
    rtShapeBox    box_inverted[NUM_SHAPE_BOX_INVERTED];
    rtShapeSphere sphere[NUM_SHAPE_SPHERE];
};
struct rtObjectRegistry
{
    rtObject obj[NUM_OBJECT];

    //int obj_plane_infinite[NUM_OBJECT_PLANE_INFINITE];
    int obj_plane_finite[NUM_OBJECT_PLANE_FINITE];
    int obj_box[NUM_OBJECT_BOX];
    int obj_box_inverted[NUM_OBJECT_BOX_INVERTED];
    int obj_sphere[NUM_OBJECT_SPHERE];
};
struct rtScene
{
    rtMaterialRegistry mat_registry;
    rtShapeRegistry    shape_registry;
    rtObjectRegistry   obj_registry;
};

// WARNING: SLOW AF ON MY LAPTOP
// Perhaps this is meant for better machines... The 2/2 option crashed my GPU but looks way better.
// This is relevant information about pushing limits and deciding the best way to go about this.
#define NUM_RAY_LAYERS_BIN_EXP     1//2//1 //< power of 2 for number of layers
#define NUM_RAY_BOUNCE_BIN_EXP     1//2//3 //< power of 2 for number of children
#define NUM_RAY_BOUNCE_TREE_DEPTH ((1<<NUM_RAY_LAYERS_BIN_EXP)-1)
#define NUM_RAY_BOUNCES_PER_LAYER ((1<<NUM_RAY_BOUNCE_BIN_EXP))
#define NUM_ENTRIES ((NUM_RAY_BOUNCES_PER_LAYER > 1) ? (((1<<(NUM_RAY_BOUNCE_BIN_EXP*(NUM_RAY_BOUNCE_TREE_DEPTH+1)))-1)/(NUM_RAY_BOUNCES_PER_LAYER-1)) : (NUM_RAY_BOUNCE_TREE_DEPTH+1))

///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
/// CONSTANTS AND HELPERS

#define M_PI_4		 0.78539816339744830961566084581988
#define M_PI_2		 1.57079632679489661923132169163980
#define M_3PI_4		 2.35619449019234492884698253745960
#define M_PI		 3.14159265358979323846264338327950
#define M_4PI_3		 4.18879020478639098461685784437270
#define M_2PI		 6.28318530717958647692528676655900
#define M_4PI		12.56637061435917295385057353311800

#define M_4_PI       1.27323954473516268615107010698010
#define M_2_PI       0.63661977236758134307553505349006
#define M_4_3PI		 0.42441318157838756205035670232670
#define M_1_PI		 0.31830988618379067153776752674503
#define M_3_4PI		 0.23873241463784300365332564505877
#define M_1_2PI		 0.15915494309189533576888376337251
#define M_1_4PI		 0.07957747154594766788444188168626

#define FLT_EPSILON 1.192092896e-07
//#define DBL_EPSILON 2.2204460492503131e-016
#define RE_EPSILON  FLT_EPSILON

#define CUBE_FACE_POSITIVE_X 0
#define CUBE_FACE_NEGATIVE_X 1
#define CUBE_FACE_POSITIVE_Y 2
#define CUBE_FACE_NEGATIVE_Y 3
#define CUBE_FACE_POSITIVE_Z 4
#define CUBE_FACE_NEGATIVE_Z 5

///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
/// PROGRAM UNIFORMS

//#define MAX_VIEWERS 1
//#define MAX_MODELS 24
struct sViewerStack
{
	mat4 projectionMat;					// projection matrix (viewer -> clip)
	mat4 projectionMatInverse;			// projection inverse matrix (clip -> viewer)
	mat4 projectionBiasMat;				// projection-bias matrix (viewer -> biased clip)
	mat4 projectionBiasMatInverse;		// projection-bias inverse matrix (biased clip -> viewer)
	mat4 viewProjectionMat;				// view-projection matrix (world -> clip)
	mat4 viewProjectionMatInverse;		// view-projection inverse matrix (clip -> world)
	mat4 viewProjectionBiasMat;			// view projection-bias matrix (world -> biased clip)
	mat4 viewProjectionBiasMatInverse;	// view-projection-bias inverse matrix (biased clip -> world)
};
struct sModelStack
{
	mat4 modelMat;						// model matrix (object -> world)
	mat4 modelMatInverse;				// model inverse matrix (world -> object)
	mat4 modelMatInverseTranspose;		// model inverse-transpose matrix (object -> world skewed)
	mat4 modelViewMat;					// model-view matrix (object -> viewer)
	mat4 modelViewMatInverse;			// model-view inverse matrix (viewer -> object)
	mat4 modelViewMatInverseTranspose;	// model-view inverse transpose matrix (object -> viewer skewed)
	mat4 modelViewProjectionMat;		// model-view-projection matrix (object -> clip)
	mat4 atlasMat;						// atlas matrix (texture -> cell)
};
uniform ubTransformStack {
	sViewerStack viewer_stack[NUM_VIEWER];
	sModelStack model_stack[NUM_MODEL];
	rtScene scene;
	int hierarchy[NUM_ENTRIES];
};

uniform vec4 uColor;
uniform vec4 uAxis;

uniform sampler2D uTex_dm;

///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
/// COMMON SETTINGS

#define USING_ANTIALIASING			1
#define USING_ANTIALIASING_RANDOM	1

///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
/// RANDOM NUMBER GENERATORS

// Random number generation: 
// https://en.wikipedia.org/wiki/List_of_random_number_generators#Pseudorandom_number_generators_(PRNGs)
// https://en.wikipedia.org/wiki/Lehmer_random_number_generator
//	-> Park-Miller
//	-> Xorshift
uint rand_seed_xorshift = 0;
void srand_xorshift(uint seed)
{
	rand_seed_xorshift = seed;
}
uint rand_xorshift()
{
	rand_seed_xorshift ^= rand_seed_xorshift << 13;
	rand_seed_xorshift ^= rand_seed_xorshift >> 17;
	rand_seed_xorshift ^= rand_seed_xorshift <<  5;
	return rand_seed_xorshift;
}

uint rand_seed_parkmiller = 0;
void srand_parkmiller(uint seed)
{
	rand_seed_parkmiller = seed;
}
uint rand_parkmiller()
{
	const uint M = 0x7fffffff;
	const uint A = 48271;
	const uint Q = M / A;
	const uint R = M % A;
	uint div = rand_seed_parkmiller / Q;
	uint rem = rand_seed_parkmiller % Q;
	int s = int(rem * A);
	int t = int(div * R);
	int result = s - t;
	if (result < 0)
		result += int(M);
	rand_seed_parkmiller = uint(result);
	return rand_seed_parkmiller;
}

const uint rand_max_open   = 1<<23;
const uint rand_max_closed = rand_max_open-1;
uint rand_seed = 0;
void srand(uint seed)
{
	rand_seed = seed;
	srand_xorshift(seed);
	srand_parkmiller(seed);
}
uint rand()
{
	rand_seed = rand_xorshift();
	rand_seed = rand_parkmiller();
	rand_seed %= rand_max_open;
	return rand_seed;
}

float randf_open()
{
	return float(rand()) / float(rand_max_open);
}
float randf_closed()
{
	return float(rand()) / float(rand_max_closed);
}
vec3 randv_open()
{
	return vec3(randf_open(), randf_open(), randf_open()) * 2.0 - 1.0;
}
vec3 randv_closed()
{
	return vec3(randf_closed(), randf_closed(), randf_closed()) * 2.0 - 1.0;
}
vec3 randv_sphere_sweep(const float sweep_azim, const float sweep_elev)
{
	float azim = radians(randf_open() * sweep_azim);
	float elev = radians(randf_open() * sweep_elev);//< If open, Z can never be extreme limit.
	float sa   = sin(azim), ca = cos(azim);
	float se   = sin(elev), ce = cos(elev);
	return vec3(se * ca, se * sa, ce);
}
vec3 randv_hemisphere()
{
	return randv_sphere_sweep(360.0, 90.0);//< If open, Z can never be 0.
}
vec3 randv_sphere()
{
	return randv_sphere_sweep(360.0, 180.0);//< If open, Z can never be -1.
}
vec3 randv_lambertian()
{
	vec3 v = randv_sphere();
	v.z += 1.0;//< Bias towards normal.
	return normalize(v);
}

///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
/// COMMON UTILITIES

// Initialize scalar.
const rtScalar rtScalar_init(const rtReal x)
{
	rtReal _x = 1.0 / x;
	return rtScalar(x, x * x, _x, _x * _x);
}

// Convert view space position to image space using main camera.
vec4 view2img(in vec4 p_view)
{
    vec4 p_clip_bias = viewer_stack[IDX_VIEWER_MAIN].projectionBiasMat * p_view;
	return p_clip_bias / p_clip_bias.w;
}

// Convert image space position to view space using main camera.
vec4 img2view(in vec4 p_img)
{
	vec4 p_view_bias = viewer_stack[IDX_VIEWER_MAIN].projectionBiasMatInverse * p_img;
	return p_view_bias / p_view_bias.w;
}

// Semi-open range test.
bool range(in rtReal x, in rtReal x_min, in rtReal x_max)
{
	return ((x >= x_min) && (x < x_max));
}

///
////////////////////////////////////////////////////////////


// Get scene object basis elements.
rtPoint rtSceneObjectPos(int uid_obj)
{
	return scene.obj_registry.obj[uid_obj].center;
}
rtVector rtSceneObjectNormal(int uid_obj)
{
	return scene.obj_registry.obj[uid_obj].normal;
}
rtVector rtSceneObjectTangent(int uid_obj)
{
	return scene.obj_registry.obj[uid_obj].tangent;
}
rtVector rtSceneObjectBitangent(int uid_obj)
{
	return scene.obj_registry.obj[uid_obj].bitangent;
}


// Ray descriptor.
struct rtRay
{
	rtPoint  origin;   //< Position of origin.
	rtVector direction;//< Direction vector of ray.
	rtScalar magnitude;//< Magnitude of direction vector.
};

// Compute position on ray.
rtPoint rtRayPoint(in rtRay ray, in rtReal param)
{
	// P(t) = P(0) + pt
	return (ray.origin + ray.direction * param);
}


// Collection of data for ray vs infinite plane test.
struct rtRayVsPlaneInfiniteData
{
	rtVector s0;
	rtReal   dot_p_normal;
	rtReal   param;
};

// Test ray vs infinite plane collision.
bool rtRayVsPlaneInfinite(out rtRayVsPlaneInfiniteData data, in rtRay ray, in rtObject obj)
{
	// Ray vs infinite plane: 
	//	0 = (P(t) - Q).n
	//	  = (P(0) + pt - Q).n
	//  s = P(0) - Q
	//	0 = (s + pt).n
	//    = (s.n) + (p.n)t
	//     -(s.n) = (p.n)t
	//  t = -(s.n)/(p.n)
	//  s' = -s = Q - P(0)
	//  t = (s'.n)/(p.n)
	// 
	// Fail if: 
	//	-> ray and normal are perpendicular 
	//		(n.p) = 0
	//	-> ray and normal are relatively aligned
	//		(n.p) > 0
	data.dot_p_normal = dot(obj.normal, ray.direction);
	if (data.dot_p_normal >= -RE_EPSILON)
		return false;
	data.s0 = obj.center - ray.origin;
	data.param = dot(obj.normal, data.s0) / data.dot_p_normal;
	return true;
}

// Compute infinite plane basis.
void rtPlaneInfiniteBasis(out rtVector normal, in rtObject obj)
{
	normal = obj.normal;
}


// Collection of data for ray vs finite plane test.
struct rtRayVsPlaneFiniteData
{
	rtRayVsPlaneInfiniteData data_infinite;

	rtPoint  position;
	rtVector st;
	rtReal   dot_s_tangent;
	rtReal   dot_s_bitangent;
};

// Test ray vs finite plane collision.
bool rtRayVsPlaneFinite(out rtRayVsPlaneFiniteData data, in rtRay ray, in rtObject obj, in rtShapePlane plane)
{
	// Ray vs finite plane: 
	// Fail if: 
	//	-> infinite plane fails
	//	-> projections of hit vector onto bases longer than half dimensions
	if (!rtRayVsPlaneInfinite(data.data_infinite, ray, obj))
		return false;
	data.position = rtRayPoint(ray, data.data_infinite.param);
	data.st = data.position - obj.center;
	data.dot_s_tangent = dot(obj.tangent, data.st);
	if (abs(data.dot_s_tangent) > plane.size_2[0])
		return false;
	data.dot_s_bitangent = dot(obj.bitangent, data.st);
	if (abs(data.dot_s_bitangent) > plane.size_2[1])
		return false;
	return true;
}

// Compute finite plane basis.
void rtPlaneFiniteBasis(out rtVector normal, out rtVector tangent, in rtObject obj)
{
	normal  = obj.normal;
	tangent = obj.tangent;
}


// Collection of data for ray vs sphere test.
struct rtRayVsSphereData
{
	rtVector s0;
	rtReal   b;
	rtReal   c;
	rtReal   d;
	rtReal   sqrt_d;
	rtReal   param_min;
	rtReal   param_max;
};

// Test ray vs sphere collision.
bool rtRayVsSphere(out rtRayVsSphereData data, in rtRay ray, in rtObject obj, in rtShapeSphere sphere)
{
	// Ray vs sphere: 
	//	r = |P(t) - Q|
	//	r^2 = |P(0) + pt - Q|^2
	//	s = P(0) - Q
	//	r^2 = (s + pt).(s + pt)
	//	0 = s.s + 2(s.p)t + (p.p)t^2 - r^2
	//	  = [p.p]t^2 + [2(s.p)]t + [s.s - r^2]
	//	a = p.p
	//	b = 2(s.p)
	//	c = s.s - r^2
	//	t = (-b +/- sqrt(b^2 - 4ac)) / 2a
	//	  = (-2(s.p) +/- sqrt(4(s.p)^2 - 4(p.p)(s.s - r^2))) / 2a
	//	  = (-(s.p) +/- sqrt((s.p)^2 - 4(p.p)(s.s - r^2))) / a
	//	s' = -s = Q - P(0)
	//	a' = 1
	//	b' = s'.p
	//	t = b' - sqrt(b'^2 - c)
	//	  = (s'.p) - sqrt((s'.p)^2 - (s.s - r^2))
	// Fail if:
	//	-> discriminant (b'^2 - c) is negative
	data.s0 = obj.center - ray.origin;
	data.b  = dot(data.s0, ray.direction);
	data.c  = dot(data.s0, data.s0) - sphere.radius.xx;
	data.d  = data.b * data.b - data.c;
	if (data.d < +RE_EPSILON)
		return false;
	data.sqrt_d = sqrt(data.d);
	data.param_min = data.b - data.sqrt_d;
	data.param_max = data.b + data.sqrt_d;
	return true;
}

// Compute sphere basis.
void rtSphereBasis(out rtVector normal, out rtVector tangent, in rtPoint position, in rtObject obj, in rtShapeSphere sphere)
{
	rtVector diff = position - obj.center;
	normal  = diff * sphere.radius._x;
	tangent = vec4(cross(obj.normal.xyz, diff.xyz), 0.0);
	tangent = (dot(tangent, tangent) <= +RE_EPSILON) ? obj.tangent : normalize(tangent);
}


// Collection of data for ray vs box test.
struct rtRayVsBoxData
{
	rtReal param_min;
	rtReal param_max;
	int cmp_min;
	int cmp_max;
};

// Test ray vs box collision.
bool rtRayVsBox(out rtRayVsBoxData data, in rtRay ray, in rtObject obj, in rtShapeBox box)
{
	// Ray vs box: 
	//	-> effectively ray vs 3 bounding slabs
	rtVector s = ray.origin - obj.center;

	// dir_box = R_box^T * dir
	vec3 dir_ray_local_inv = 1.0 / vec3(
		dot(ray.direction, obj.tangent),
		dot(ray.direction, obj.bitangent),
		dot(ray.direction, obj.normal)
	);
	// pos_box = T_box^-1 * pos
	vec3 pos_ray_local = vec3(
		dot(s, obj.tangent),
		dot(s, obj.bitangent),
		dot(s, obj.normal)
	);

	// Which side of each plane are we on.
	int psgn_x = (dir_ray_local_inv.x >= 0.0) ? 1 : 0, nsgn_x = 1 - psgn_x;
	int psgn_y = (dir_ray_local_inv.y >= 0.0) ? 1 : 0, nsgn_y = 1 - psgn_y;
	int psgn_z = (dir_ray_local_inv.z >= 0.0) ? 1 : 0, nsgn_z = 1 - psgn_z;
	
	int cmp0 = CUBE_FACE_POSITIVE_X + psgn_x;
	int cmp1 = CUBE_FACE_POSITIVE_X + nsgn_x;
	rtReal tmin  = (box.bounds_2[psgn_x].x - pos_ray_local.x) * dir_ray_local_inv.x;
	rtReal tmax  = (box.bounds_2[nsgn_x].x - pos_ray_local.x) * dir_ray_local_inv.x;
	rtReal tcmin = (box.bounds_2[psgn_y].y - pos_ray_local.y) * dir_ray_local_inv.y;
	rtReal tcmax = (box.bounds_2[nsgn_y].y - pos_ray_local.y) * dir_ray_local_inv.y;

	if ((tmin > tcmax) || (tcmin > tmax))
		return false;

	if (tmin < tcmin)
	{
		tmin = tcmin;
		cmp0 = CUBE_FACE_POSITIVE_Y + psgn_y;
	}
	if (tmax > tcmax)
	{
		tmax = tcmax;
		cmp1 = CUBE_FACE_POSITIVE_Y + nsgn_y;
	}

	tcmin = (box.bounds_2[psgn_z].z - pos_ray_local.z) * dir_ray_local_inv.z;
	tcmax = (box.bounds_2[nsgn_z].z - pos_ray_local.z) * dir_ray_local_inv.z;

	if ((tmin > tcmax) || (tcmin > tmax))
		return false;

	if (tmin < tcmin)
	{
		tmin = tcmin;
		cmp0 = CUBE_FACE_POSITIVE_Z + psgn_z;
	}
	if (tmax > tcmax)
	{
		tmax = tcmax;
		cmp1 = CUBE_FACE_POSITIVE_Z + nsgn_z;
	}

	data.param_min = tmin;
	data.param_max = tmax;

	data.cmp_min = cmp0;
	data.cmp_max = cmp1;

	return true;
}

// Compute box basis.
void rtBoxBasis(out rtVector normal, out rtVector tangent, in int cmp, in rtObject obj)
{
	switch(cmp)
	{
	case CUBE_FACE_POSITIVE_X:
	{
		normal  = +obj.tangent;
		tangent = +obj.bitangent;
	}	break;
	case CUBE_FACE_NEGATIVE_X:
	{
		normal  = -obj.tangent;
		tangent = -obj.bitangent;
	}	break;
	case CUBE_FACE_POSITIVE_Y:
	{
		normal  = +obj.bitangent;
		tangent = -obj.tangent;
	}	break;
	case CUBE_FACE_NEGATIVE_Y:
	{
		normal  = -obj.bitangent;
		tangent = +obj.tangent;
	}	break;
	case CUBE_FACE_POSITIVE_Z:
	{
		normal  = +obj.normal;
		tangent = +obj.tangent;
	}	break;
	case CUBE_FACE_NEGATIVE_Z:
	{
		normal  = -obj.normal;
		tangent = -obj.tangent;
	}	break;
	default:
	{
	}	break;
	}
}


const rtReal ray_param_min = 0.00001;
const rtReal ray_param_max = 10000.0;

// Utility descriptor to track ray hits.
struct rtRayHitTracker
{
	rtReal param;//< Ray parameter; smallest positive wins.
	int uid_obj; //< Unique identifier of hit object.
	int idx_cmp; //< Index of object component hit.
};
rtRayHitTracker tracker_default()
{
	return rtRayHitTracker(ray_param_max, -1, -1);
}
bool tracker_valid(in rtRayHitTracker tracker)
{
	return (tracker.uid_obj >= 0);
}

// Utility descriptor to store ray hit solution.
struct rtRayHitSolution
{
	rtPoint  position;//< Position of hit.
	rtVector normal;  //< Normal at hit position.
	rtVector tangent; //< Tangent at hit position.
	rtVector vec_in;  //< Inbound vector.
	rtReal   dot_in;  //< Dot product of inbound vector with normal.
	mat3 basis;       //< Final basis at hit position.
	int uid_obj;      //< Unique identifier of hit object.
};
void solution_invalidate(inout rtRayHitSolution solution)
{
	solution.uid_obj = -1;
}
bool solution_valid(in rtRayHitSolution solution)
{
	return (solution.uid_obj >= 0);
}

// Compute basis of hemisphere.
mat3 generate_hemisphere_basis(in vec3 incident, in vec3 normal, in vec3 tangent_alt, in float dot_incident_normal)
{
	// Flatten tangent; if it becomes zero, use alternate.
	// This happens if it is parallel to the normal.
	vec3 tangent = incident - dot_incident_normal * normal;
	tangent = (dot(tangent, tangent) > RE_EPSILON) ? normalize(tangent) : tangent_alt;
	vec3 bitangent = cross(normal, tangent);
	return mat3(tangent, bitangent, normal);
}

bool rtRayVsScene(inout rtRayHitSolution solution, in rtRay ray)
{
	rtRayHitTracker tracker = tracker_default();

	rtRayVsPlaneFiniteData[NUM_SHAPE_PLANE_FINITE] data_plane_finite;
	rtRayVsBoxData[NUM_SHAPE_BOX_INVERTED]         data_box_inverted;
	rtRayVsBoxData[NUM_SHAPE_BOX]                  data_box;
	rtRayVsSphereData[NUM_SHAPE_SPHERE]            data_sphere;
	
	// Invalidate solution.
	solution_invalidate(solution);

	// Test shapes.
	int i;

	for (i = 0; i < NUM_OBJECT_PLANE_FINITE; ++i)
	{
		int uid_obj   = scene.obj_registry.obj_plane_finite[i];
		int idx_shape = scene.obj_registry.obj[uid_obj].idx_shape;
		if (!rtRayVsPlaneFinite(data_plane_finite[idx_shape], ray, scene.obj_registry.obj[uid_obj], scene.shape_registry.plane_finite[idx_shape]))
			continue;
		rtReal param = data_plane_finite[idx_shape].data_infinite.param;
		if (!range(param, ray_param_min, tracker.param))
			continue;
		tracker.uid_obj = uid_obj;
		tracker.param   = param;
	}

	for (i = 0; i < NUM_OBJECT_BOX_INVERTED; ++i)
	{
		int uid_obj   = scene.obj_registry.obj_box_inverted[i];
		int idx_shape = scene.obj_registry.obj[uid_obj].idx_shape;
		if (!rtRayVsBox(data_box_inverted[idx_shape], ray, scene.obj_registry.obj[uid_obj], scene.shape_registry.box_inverted[idx_shape]))
			continue;
		rtReal param = data_box_inverted[idx_shape].param_max;
		if (!range(param, ray_param_min, tracker.param))
			continue;
		tracker.uid_obj = uid_obj;
		tracker.param   = param;
		tracker.idx_cmp = data_box_inverted[idx_shape].cmp_max;
	}

	for (i = 0; i < NUM_OBJECT_BOX; ++i)
	{
		int uid_obj   = scene.obj_registry.obj_box[i];
		int idx_shape = scene.obj_registry.obj[uid_obj].idx_shape;
		if (!rtRayVsBox(data_box[idx_shape], ray, scene.obj_registry.obj[uid_obj], scene.shape_registry.box[idx_shape]))
			continue;
		rtReal param = data_box[idx_shape].param_min;
		if (!range(param, ray_param_min, tracker.param))
			continue;
		tracker.uid_obj = uid_obj;
		tracker.param   = param;
		tracker.idx_cmp = data_box[idx_shape].cmp_min;
	}

	for (i = 0; i < NUM_OBJECT_SPHERE; ++i)
	{
		int uid_obj   = scene.obj_registry.obj_sphere[i];
		int idx_shape = scene.obj_registry.obj[uid_obj].idx_shape;
		if (!rtRayVsSphere(data_sphere[idx_shape], ray, scene.obj_registry.obj[uid_obj], scene.shape_registry.sphere[idx_shape]))
			continue;
		rtReal param = data_sphere[idx_shape].param_min;
		if (!range(param, ray_param_min, tracker.param))
			continue;
		tracker.uid_obj = uid_obj;
		tracker.param   = param;
	}

	// Bail if tracker invalid (nothing hit).
	if (!tracker_valid(tracker))
		return false;

	// Complete solution.
	switch(scene.obj_registry.obj[tracker.uid_obj].type_shape)
	{
	case IDX_SHAPE_TYPE_PLANE_FINITE:
	{
		int idx_shape = scene.obj_registry.obj[tracker.uid_obj].idx_shape;
		solution.uid_obj  = tracker.uid_obj;
		solution.position = data_plane_finite[idx_shape].position;
		rtPlaneFiniteBasis(solution.normal, solution.tangent, scene.obj_registry.obj[tracker.uid_obj]);
		solution.dot_in = data_plane_finite[idx_shape].data_infinite.dot_p_normal;
	} break;
	case IDX_SHAPE_TYPE_BOX_INVERTED:
	{
		solution.uid_obj  = tracker.uid_obj;
		solution.position = rtRayPoint(ray, tracker.param);
		rtBoxBasis(solution.normal, solution.tangent, tracker.idx_cmp, scene.obj_registry.obj[tracker.uid_obj]);
		solution.normal  = -solution.normal;
		solution.tangent = -solution.tangent;
		solution.dot_in  = dot(solution.normal, ray.direction);
	} break;
	case IDX_SHAPE_TYPE_BOX:
	{
		solution.uid_obj  = tracker.uid_obj;
		solution.position = rtRayPoint(ray, tracker.param);
		rtBoxBasis(solution.normal, solution.tangent, tracker.idx_cmp, scene.obj_registry.obj[tracker.uid_obj]);
		solution.dot_in = dot(solution.normal, ray.direction);
	} break;
	case IDX_SHAPE_TYPE_SPHERE:
	{
		int idx_shape = scene.obj_registry.obj[tracker.uid_obj].idx_shape;
		solution.uid_obj  = tracker.uid_obj;
		solution.position = rtRayPoint(ray, tracker.param);
		rtSphereBasis(solution.normal, solution.tangent, solution.position, scene.obj_registry.obj[tracker.uid_obj], scene.shape_registry.sphere[idx_shape]);
		solution.dot_in = dot(solution.normal, ray.direction);
	} break;
	default:
	{
	} break;
	}
	
	// Bail if solution invalid (unhandled case).
	if (!solution_valid(solution))
		return false;

	// Compute final basis.
	solution.basis = generate_hemisphere_basis(
		ray.direction.xyz,
		solution.normal.xyz,
		solution.tangent.xyz,
		solution.dot_in
	);

	// Store inbound.
	solution.vec_in = ray.direction;

	// Success (hit is good).
	return true;
}


struct rtPathEntry
{
	// This entry: 
	//                  n      /
	//  v               x   l /
	//  <---- - - - - - ---->O
	//                      /p
	//                     /
	// 

	// Each child ray contributes color to parent observer.
	// Goal is to compute radiance: 
	//	No participating media: 
	//		L_in(c,-v) = L_out(p,v)
	//	Full rendering equation: 
	//		L_out(p,v) = L_emit(p,v) + S[l>>Om]( f(l,v)*L_in(p,l)*dot(n,l) )dl
	//		L_in(p,l)  = L_out(ray(p,l),-l)
	// where
	//	p = location of this point on surface
	//	n = normal at this point on surface
	//	v = outbound view direction (direction towards observer)
	//	l = inbound light direction (direction towards light source)
	rtColor L_out;//< outbound radiance

	// Emittance
	rtColor L_emit;

	// Position on surface being evaluated
	rtPoint p;

	// Normal at evaluated point.
	rtVector n;

	// Vector towards receiver or observer
	// (may be camera or previous surface)
	//	-> same as negative inbound ray direction
	//	-> same for all sibling rays
	rtVector v;//< This towards parent

	// Generated light vector towards light source
	// (may be actual light or next surface)
	//	-> same as outbound ray direction
	//	-> likely different for all sibling rays
	rtVector l;//< Parent towards this

	// BRDF
	rtColor brdf;

	// Lambertian coefficient
	rtReal kd;

	// Hit material.
	int type_mat;
	int idx_mat;

	// Ray depth
	int depth;

	// Invalid depth
	int depth_invalid;
};
void entry_reset(inout rtPathEntry entry, in rtRay ray, in int depth_parent)
{
	// Regardless of entry status, start with zero radiance.
	entry.L_out = rtColor(0.0);

	// Describe vectors from source and towards target.
	entry.v = -ray.direction;
	entry.l = +ray.direction;

	// Describe path depth.
	entry.depth = 1 + depth_parent;
}
void entry_invalidate(inout rtPathEntry entry, in int depth_invalid_parent)
{
	// Zero normal, nothing hit.
	entry.n = vec4(0.0);
	entry.type_mat = IDX_MATERIAL_TYPE_DEFAULT;//-1;
	entry.idx_mat  = IDX_MATERIAL_DEFAULT_GRAY_EMIT;//-1;

	// Increment bad path depth.
	entry.depth_invalid = 1 + depth_invalid_parent;
}
void entry_update(inout rtPathEntry entry, in rtRayHitSolution solution)
{
	// Set hit point results.
	int uid_obj  = solution.uid_obj;
	entry.p = solution.position;
	entry.n = vec4(solution.basis[2], 0.0);
	entry.type_mat = scene.obj_registry.obj[uid_obj].type_mat;
	entry.idx_mat  = scene.obj_registry.obj[uid_obj].idx_mat;

	// Valid.
	entry.depth_invalid = 0;
}

rtReal lambertian_evaluate(in rtVector n, in rtVector l)
{
	rtReal kd = dot(n, l);
	kd = max(kd, 0.0);
	return kd;
}
rtReal entry_evaluate_lambertian(in rtPathEntry entry_parent, in rtPathEntry entry)
{
	return lambertian_evaluate(entry_parent.n, entry.l);
}

rtColor brdf_evaluate_default(in int idx_mat,
	in rtPoint p, in rtVector n, in rtVector v, in rtVector l
)
{
	rtColor brdf = scene.mat_registry.mat_default[idx_mat].albedo;
	brdf *= M_1_PI;
	return brdf;
}
rtColor entry_evaluate_brdf_default(in rtPathEntry entry_parent, in rtPathEntry entry)
{
	return brdf_evaluate_default(entry_parent.idx_mat, entry_parent.p, entry_parent.n, entry_parent.v, entry.l);
}

void entry_integrate(inout rtPathEntry entry)
{
	// Compute emittance.
	entry.L_emit = scene.mat_registry.mat_default[entry.idx_mat].emissive;

	// Finish integral.
	entry.L_out *= M_PI;
	entry.L_out += entry.L_emit;
}

void entry_evaluate(inout rtPathEntry entry_parent, inout rtPathEntry entry)
{
	// Lambertian product: 
	// Compare this entry's light vector against parent's normal.
	entry.kd = entry_evaluate_lambertian(entry_parent, entry);
	
	// BRDF: 
	// Dependent on this light vector and parent's (view).
	entry.brdf = entry_evaluate_brdf_default(entry_parent, entry);

	// Integral:
	// This entry contributes to parent's integral.
	// Parent adds this evaluated BRDF and Lambertian product combined with radiance.
	entry_parent.L_out += entry.brdf * entry.kd * entry.L_out;
}


void entry_debug(inout rtPathEntry entry)
{
	//// DEBUG: visualize randomness.
	//entry.L_out.rgb = randv_open() * 0.5 + 0.5;
	//entry.L_out.rgb = vec3(randf_open()) * 0.5 + 0.5;

	//// DEBUG: visualize vectors.
	//entry.L_out = normalize(entry.l) * 0.5 + 0.5;
	//entry.L_out = normalize(entry.n) * 0.5 + 0.5;
	
	//// DEBUG: visualize material.
	//entry.L_out = scene.mat_registry.mat_default[entry.idx_mat].albedo;
	//entry.L_out = scene.mat_registry.mat_default[entry.idx_mat].emissive;

	//// DEBUG: visualize integral components.
	//entry.L_out.rgb = vec3(entry.kd);
	//entry.L_out = entry.brdf;

	// Solid alpha.
	entry.L_out.a = 1.0;
}

void output_final(out vec4 color_out, out float depth_out, in rtPathEntry entry)
{
	color_out = entry.L_out;
	depth_out = view2img(entry.p).z;
}

void trace_ray_gi(out vec4 color_out, out float depth_out, in rtRay ray)
{
	rtRayHitSolution[NUM_ENTRIES] solution;
	rtPathEntry[NUM_ENTRIES] entry;
	
	int idx = 0;
	int idx_parent = -1;

	vec3 light_vec;

	// Require first hit.
	entry_reset(entry[idx], ray, -1);
	if (!rtRayVsScene(solution[idx], ray))
	{
		color_out = vec4(0.0);
		depth_out = 1.0;
		return;
	}
	entry_update(entry[idx], solution[idx]);

	//// DEBUG: pre-solve.
	//entry_debug(entry[idx]);
	//output_final(color_out, depth_out, entry[0]);
	//return;

	// Iterate through paths and compute bounces.
	for (++idx; idx < NUM_ENTRIES; ++idx)
	{
		idx_parent = hierarchy[idx];

		// If parent entry is not valid, neither is this one.
		if (!solution_valid(solution[idx_parent]))
		{
			// Subsequent invalid.
			solution_invalidate(solution[idx]);
			entry_invalidate(entry[idx], entry[idx_parent].depth_invalid);
			continue;
		}

		// Use parent hemisphere basis to direct new ray.
		// Choose scattering model here (hemisphere, Lambertian).
		light_vec = randv_lambertian();
		light_vec = solution[idx_parent].basis * light_vec;

		// Generate new ray.
		ray = rtRay(
			solution[idx_parent].position,
			rtVector(light_vec, 0.0),
			rtScalar_init(1.0)
		);
		
		// Send new ray.
		entry_reset(entry[idx], ray, entry[idx_parent].depth);
		if (!rtRayVsScene(solution[idx], ray))
		{
			// Invalidate first.
			entry_invalidate(entry[idx], entry[idx_parent].depth_invalid);
			continue;
		}
		entry_update(entry[idx], solution[idx]);
	}

	// Evaluate results.
	for (--idx; idx > 0; --idx)
	{
		// Skip deep failures.
		if (entry[idx].depth_invalid > 1)
			continue;

		idx_parent = hierarchy[idx];
		entry_integrate(entry[idx]);
		entry_evaluate(entry[idx_parent], entry[idx]);
	}

	// Root does not evaluate BRDF or Lambertian; ray originated
	// from camera, not off of a surface; receiver is camera.
	// Final entry may have emittance.
	entry_integrate(entry[idx]);
	entry[idx].L_out.a = 1.0;

	// Output final result of first hit.
	output_final(color_out, depth_out, entry[0]);

	//// DEBUG: post-solve.
	//entry_debug(entry[0]);
	//output_final(color_out, depth_out, entry[0]);
}


// Generate random ray given lens and focal plane.
// Originate ray from imaginary lens at camera origin towards focal plane.
rtRay camera_focal(const vec3 fragCoord, const rtPoint focal)
{
	const bool using_random_pos_lens  = true;
	const bool using_random_pos_pixel = true;

	const rtReal lens_radius  = using_random_pos_lens  ? 1.0 : 0.0;
	const rtReal pixel_scale  = using_random_pos_pixel ? 1.0 : 0.0;

	const rtReal lens_dist    = 0.0;
	const rtReal focal_offset = 0.0;
	const rtReal focal_dist   = abs(focal.y) + focal_offset;

	// Radial sample on lens.
	rtReal angle  = radians(randf_open() * 360.0);
	rtReal radial = randf_open() * lens_radius;
	vec2 lens_offset = vec2(cos(angle), sin(angle)) * radial;
	vec4 pos_lens_view = vec4(lens_offset.x, lens_dist, lens_offset.y, 1.0);

	// Pixel sample on focal plane.
	vec2 pixel_offset = (vec2(randf_open(), randf_open()) - 0.5) * pixel_scale;
	vec4 pos_pixel_img = vec4((fragCoord.xy + pixel_offset) * uAxis.zw, fragCoord.z, 1.0);
	vec4 pos_pixel_view = img2view(pos_pixel_img);
	pos_pixel_view.xyz *= focal_dist / pos_pixel_view.y;

	// Generate ray.
	vec4 ray_origin    = pos_lens_view;
	vec4 ray_direction = normalize(pos_pixel_view - pos_lens_view);
	return rtRay(ray_origin, ray_direction, rtScalar_init(1.0));
}

// Generate ray given internal image plane and pinhole.
// The effect is likely too small to notice any antialiasing.
rtRay camera_pinhole(const vec3 fragCoord, const rtReal filmDist)
{
	const bool using_random_pos_pinhole = true;
	const bool using_random_pos_film    = true;

	const rtReal pinhole_radius   = using_random_pos_pinhole ? 0.005 : 0.0;
	const rtReal film_pixel_scale = using_random_pos_film    ? 1.000 : 0.0;
	
	const rtReal film_plane_scale = 2.0;
	const rtReal focal_offset     = 0.0;
	const rtReal focal_dist       = -(abs(filmDist) + focal_offset);

	// Radial sample on lens.
	rtReal angle  = radians(randf_open() * 360.0);
	rtReal radial = randf_open() * pinhole_radius;
	vec2 pinhole_offset = vec2(cos(angle), sin(angle)) * radial;
	vec4 pos_pinhole_view = vec4(pinhole_offset.x, 0.0, pinhole_offset.y, 1.0);

	// Pixel sample on focal plane.
	vec2 film_offset = (vec2(randf_open(), randf_open()) - 0.5) * film_pixel_scale;
	vec4 pos_film_img = vec4((fragCoord.xy + film_offset) * uAxis.zw, 0.0, 1.0);
	vec4 pos_film_view = img2view(pos_film_img);
	pos_film_view.xyz *= focal_dist / pos_film_view.y;//< inversion correction
	//pos_film_view.xz  *= film_plane_scale;//< pinhole effect - looks strange
	//pos_film_view.y    = focal_dist;	  //< pinhole distance

	// Generate ray.
	vec4 ray_origin    = pos_film_view;
	vec4 ray_direction = normalize(pos_pinhole_view - pos_film_view);
	return rtRay(ray_origin, ray_direction, rtScalar_init(1.0));
}

void main_pathtracing_gi(out vec4 fragColor, out float fragDepth, const vec3 fragCoord)
{
	// Default values.
	fragColor = uColor * texture(uTex_dm, vTexcoord_atlas.xy);
	fragDepth = fragCoord.z;

	// Seed random.
	srand(uint(dot(gl_FragCoord, gl_FragCoord)));

	// Position of room for focus.
	rtPoint pos_room_view = rtSceneObjectPos(UID_OBJECT_ROOM);
	const int msaa_kernel_size = 16;

	vec4  color_accum = vec4(0.0), color;
	float depth_accum = 0.0, depth;
	for (int i = 0; i < msaa_kernel_size; ++i)
	{
		rtRay ray = camera_focal(fragCoord, pos_room_view);

		// Trace ray.
		trace_ray_gi(color, depth, ray);
		color_accum += color;
		depth_accum += depth;
	}
	color_accum /= msaa_kernel_size;
	depth_accum /= msaa_kernel_size;

	// Gamma-corrected output.
	fragColor = sqrt(color_accum);
	fragDepth = depth_accum;
}


void main()
{
	main_pathtracing_gi(rtFragColor, gl_FragDepth, gl_FragCoord.xyz);
}
