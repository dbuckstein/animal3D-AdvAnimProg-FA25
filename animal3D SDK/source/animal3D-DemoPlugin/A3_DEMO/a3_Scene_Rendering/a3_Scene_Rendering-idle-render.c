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

	a3_Scene_Rendering-idle-render.c/.cpp
	Demo mode implementations: rendering scene.

	********************************************
	*** RENDERING FOR RENDERING SCENE MODE   ***
	********************************************
*/

//-----------------------------------------------------------------------------

#include "../a3_Scene_Rendering.h"

#include "../a3_DemoState.h"

#include "../_a3_scene_utilities/a3_SceneRenderUtils.h"


// OpenGL
#ifdef _WIN32
#include <gl/glew.h>
#include <Windows.h>
#include <GL/GL.h>
#else	// !_WIN32
#include <OpenGL/gl3.h>
#endif	// _WIN32


//-----------------------------------------------------------------------------

// controls for pipelines mode
void a3rendering_render_controls(a3_DemoState const* demoState, a3_Scene_Rendering const* scene,
	a3_TextRenderer const* text, a3vec4 const col,
	a3f32 const textAlign, a3f32 const textDepth, a3f32 const textOffsetDelta, a3f32 textOffset)
{
	// display mode info
	a3byte const* pipelineText[rendering_pipeline_max] = {
		"Forward rendering",
	};

	// forward pipeline names
	a3byte const* renderProgramName[rendering_render_max] = {
		"Solid color",
		"Texture",
		"Lambert shading",
		"Phong shading",
		"Ray-tracing",
	};

	// forward display names
	a3byte const* displayProgramName[rendering_display_max] = {
		"Texture",
	};

	// active camera name
	a3byte const* cameraText[rendering_camera_max] = {
		"rendering scene camera",
	};

	// constant color target names
	a3byte const colorBufferText[] = "Color target 0: FINAL DISPLAY COLOR";
	// constant depth target name
	a3byte const depthBufferText[] = "Depth buffer";

	// pass names
	a3byte const* passName[rendering_pass_max] = {
		"Pass: Render scene objects",
		"Pass: Composite",
	};
	a3byte const* targetText_scene[rendering_target_scene_max] = {
		colorBufferText,
		depthBufferText,
	};
	a3byte const* targetText_composite[rendering_target_scene_max] = {
		colorBufferText,
	};
	a3byte const* const* targetText[rendering_pass_max] = {
		targetText_scene,
		targetText_composite,
	};

	// pipeline and target
	a3_Scene_Rendering_RenderProgramName const render = scene->render;
	a3_Scene_Rendering_DisplayProgramName const display = scene->display;
	a3_Scene_Rendering_ActiveCameraName const activeCamera = scene->activeCamera;
	a3_Scene_Rendering_PipelineName const pipeline = scene->pipeline;
	a3_Scene_Rendering_PassName const pass = scene->pass;
	a3_Scene_Rendering_TargetName const targetIndex = scene->targetIndex[pass];
	a3_Scene_Rendering_TargetName const targetCount = scene->targetCount[pass];

	// demo modes
	a3textDraw(text, textAlign, textOffset += textOffsetDelta, textDepth, col.r, col.g, col.b, col.a,
		"    Pipeline (%u / %u) ('[' | ']'): %s", pipeline + 1, rendering_pipeline_max, pipelineText[pipeline]);
	a3textDraw(text, textAlign, textOffset += textOffsetDelta, textDepth, col.r, col.g, col.b, col.a,
		"    Display pass (%u / %u) ('(' | ')'): %s", pass + 1, rendering_pass_max, passName[pass]);
	a3textDraw(text, textAlign, textOffset += textOffsetDelta, textDepth, col.r, col.g, col.b, col.a,
		"        Target (%u / %u) ('{' | '}'): %s", targetIndex + 1, targetCount, targetText[pass][targetIndex]);

    // lighting modes
    a3textDraw(text, textAlign, textOffset += textOffsetDelta, textDepth, col.r, col.g, col.b, col.a,
        "    Rendering mode (%u / %u) ('j' | 'k'): %s", render + 1, rendering_render_max, renderProgramName[render]);
    a3textDraw(text, textAlign, textOffset += textOffsetDelta, textDepth, col.r, col.g, col.b, col.a,
        "    Display mode (%u / %u) ('J' | 'K'): %s", display + 1, rendering_display_max, displayProgramName[display]);
    a3textDraw(text, textAlign, textOffset += textOffsetDelta, textDepth, col.r, col.g, col.b, col.a,
        "    Active camera (%u / %u) ('c' prev | next 'v'): %s", activeCamera + 1, rendering_camera_max, cameraText[activeCamera]);

    // tests
    a3textDraw(text, textAlign, textOffset += textOffsetDelta, textDepth, col.r, col.g, col.b, col.a,
        "    Test ray fired: %s", (scene->test_ray_fired ? "TRUE " : "FALSE"));
    a3textDraw(text, textAlign, textOffset += textOffsetDelta, textDepth, col.r, col.g, col.b, col.a,
        "        Hit: %s; t=%lf", (scene->test_ray_hit ? "TRUE " : "FALSE"), (a3f64)scene->test_ray_param);
}


//-----------------------------------------------------------------------------

//void a3demo_uploadTransformStacks(
//    a3_UniformBuffer const* ubo_transform_stacks,
//    a3_SceneModelMatrixStack const* model_matrix_stacks, a3_SceneViewerMatrixStack const* viewer_matrix_stacks,
//    a3ui32 const max_models, a3ui32 const num_models, a3ui32 const max_viewers, a3ui32 const num_viewers);


////////////////////////////////////////////////////////////////////////////////
/// COPY BELOW TO SHADER, REMOVE INCOMPATIBLE TERMS, ADD SCENE TO UBO
////////////////////////////////////////////////////////////////////////////////

typedef a3real rtReal;
typedef struct rtScalar
{
    rtReal x;
    rtReal xx;
    rtReal _x;
    rtReal _xx;
} rtScalar;
typedef a3vec4 rtVector;
typedef a3vec4 rtPoint;
typedef a3vec4 rtColor;


// RAY-TRACING STUFF
typedef struct rtShapePlane
{
    rtVector size_2;//width, height, pad[2]
} rtShapePlane;
typedef struct rtShapeBox
{
    rtVector bounds_2[2];//positive width/height/depth/pad, negative
} rtShapeBox;
typedef struct rtShapeSphere
{
    rtScalar radius;
} rtShapeSphere;
typedef struct rtMaterialDefault
{
    rtColor albedo;
    rtColor emissive;
} rtMaterialDefault;
typedef struct rtObject
{
    rtPoint center;
    rtVector normal;
    rtVector tangent;
    rtVector bitangent;

    int type_shape;
    int idx_shape;
    int type_mat;
    int idx_mat;
} rtObject;

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

typedef struct rtMaterialRegistry
{
    rtMaterialDefault mat_default[NUM_MATERIAL_DEFAULT];
} rtMaterialRegistry;
typedef struct rtShapeRegistry
{
    //rtShapePlane  plane_infinite[NUM_SHAPE_PLANE_INFINITE];
    rtShapePlane  plane_finite[NUM_SHAPE_PLANE_FINITE];
    rtShapeBox    box[NUM_SHAPE_BOX];
    rtShapeBox    box_inverted[NUM_SHAPE_BOX_INVERTED];
    rtShapeSphere sphere[NUM_SHAPE_SPHERE];
} rtShapeRegistry;
typedef struct rtObjectRegistry
{
    rtObject obj[NUM_OBJECT];

    //int obj_plane_infinite[NUM_OBJECT_PLANE_INFINITE];
    int obj_plane_finite[NUM_OBJECT_PLANE_FINITE];
    int obj_box[NUM_OBJECT_BOX];
    int obj_box_inverted[NUM_OBJECT_BOX_INVERTED];
    int obj_sphere[NUM_OBJECT_SPHERE];
} rtObjectRegistry;
typedef struct rtScene
{
    rtMaterialRegistry mat_registry;
    rtShapeRegistry    shape_registry;
    rtObjectRegistry   obj_registry;
} rtScene;

////////////////////////////////////////////////////////////////////////////////
/// STOP HERE
////////////////////////////////////////////////////////////////////////////////


void rtScalarInit(rtScalar* const scalar, a3real const x)
{
    scalar->x  = x;
    scalar->xx = x * x;
    if (scalar->xx >= a3real_epsilon)
    {
        scalar->_x  = a3recip(scalar->x);
        scalar->_xx = a3recip(scalar->xx);
    }
    else
    {
        scalar->_x  = a3real_zero;
        scalar->_xx = a3real_zero;
    }
}


// Breadth-first traversal hierarchy.
// Number of nodes: N = (k^(h+1))/(k-1) when k>1, otherwise (h+1)
//  k = number of children per node
//  h = tree height (distance from leaf to root)
// Keep it constant for optimization!
#define NUM_RAY_LAYERS_BIN_EXP     1//2//1 //< power of 2 for number of layers
#define NUM_RAY_BOUNCE_BIN_EXP     1//2//3 //< power of 2 for number of children
#define NUM_RAY_BOUNCE_TREE_DEPTH ((1<<NUM_RAY_LAYERS_BIN_EXP)-1)
#define NUM_RAY_BOUNCES_PER_LAYER ((1<<NUM_RAY_BOUNCE_BIN_EXP))
#define NUM_ENTRIES ((NUM_RAY_BOUNCES_PER_LAYER > 1) ? (((1<<(NUM_RAY_BOUNCE_BIN_EXP*(NUM_RAY_BOUNCE_TREE_DEPTH+1)))-1)/(NUM_RAY_BOUNCES_PER_LAYER-1)) : (NUM_RAY_BOUNCE_TREE_DEPTH+1))
static int rtHierarchy[NUM_ENTRIES];
static a3boolean rtBuildHierarchy(void)
{
    int num_entries = 0;
    int idx_entry = 0;
    int idx_parent_entry = -1;

    int num_clusters_this_layer = 1;
    int idx_layer;
    int idx_cluster_this_layer;
    int idx_entry_this_cluster;

    // Fixed root.
    rtHierarchy[idx_entry++] = idx_parent_entry;

    // Same as incrementing parent index every k iterations.
    // Add counter for validation.
    num_entries += num_clusters_this_layer;
    for (idx_layer = 0; idx_layer < NUM_RAY_BOUNCE_TREE_DEPTH; ++idx_layer)
    {
        for (idx_cluster_this_layer = 0; idx_cluster_this_layer < num_clusters_this_layer; ++idx_cluster_this_layer)
        {
            ++idx_parent_entry;
            for (idx_entry_this_cluster = 0; idx_entry_this_cluster < NUM_RAY_BOUNCES_PER_LAYER; ++idx_entry_this_cluster)
            {
                rtHierarchy[idx_entry++] = idx_parent_entry;
            }
        }
        num_clusters_this_layer *= NUM_RAY_BOUNCES_PER_LAYER;
        num_entries += num_clusters_this_layer;
    }
    if (idx_entry != NUM_ENTRIES)
        return a3false;
    if (num_entries != NUM_ENTRIES)
        return a3false;
    return a3true;
}


static a3ui8 transform_stack_buffer[1<<16];
static void a3demo_uploadTransformStacks(
    a3_UniformBuffer const* ubo_transform_stacks,
    a3_SceneModelMatrixStack const* model_matrix_stacks, a3ui32 const num_models,
    a3_SceneViewerMatrixStack const* viewer_matrix_stacks, a3ui32 const num_viewers,
    rtScene const* scene_raytracing
)
{
    a3ui32 const viewers_size = num_viewers * sizeof(a3_SceneViewerMatrixStack);
    a3ui32 const models_size = num_models * sizeof(a3_SceneModelMatrixStack);
    a3ui32 const scene_size = 1 * sizeof(rtScene);
    a3ui32 const hierarchy_size = sizeof(rtHierarchy);
    a3ui32 written = 0;
    memcpy(&transform_stack_buffer[written], viewer_matrix_stacks, viewers_size);
    written += viewers_size;
    memcpy(&transform_stack_buffer[written], model_matrix_stacks, models_size);
    written += models_size;
    memcpy(&transform_stack_buffer[written], scene_raytracing, scene_size);
    written += scene_size;
    memcpy(&transform_stack_buffer[written], rtHierarchy, hierarchy_size);
    written += hierarchy_size;
    a3bufferFixedRefill(ubo_transform_stacks, 0, written, transform_stack_buffer);
}


void rtBuildScene(rtScene* const scene, a3_SceneObject const* const scene_object_base, a3_SceneModelMatrixStack const* const scene_model)
{
    rtVector tmp = a3vec4_zero;

    int num_objects_plane_finite = 0;
    int num_objects_box          = 0;
    int num_objects_box_inverted = 0;
    int num_objects_sphere       = 0;

    // Clear.
    memset(scene, 0x00, sizeof(rtScene));
    
    // Materials:
    rtMaterialDefault* const material_default_red        = &scene->mat_registry.mat_default[IDX_MATERIAL_DEFAULT_RED];
    rtMaterialDefault* const material_default_yellow     = &scene->mat_registry.mat_default[IDX_MATERIAL_DEFAULT_YELLOW];
    rtMaterialDefault* const material_default_green      = &scene->mat_registry.mat_default[IDX_MATERIAL_DEFAULT_GREEN];
    rtMaterialDefault* const material_default_cyan       = &scene->mat_registry.mat_default[IDX_MATERIAL_DEFAULT_CYAN];
    rtMaterialDefault* const material_default_blue       = &scene->mat_registry.mat_default[IDX_MATERIAL_DEFAULT_BLUE];
    rtMaterialDefault* const material_default_magenta    = &scene->mat_registry.mat_default[IDX_MATERIAL_DEFAULT_MAGENTA];
    rtMaterialDefault* const material_default_white_emit = &scene->mat_registry.mat_default[IDX_MATERIAL_DEFAULT_WHITE_EMIT];
    rtMaterialDefault* const material_default_gray_emit  = &scene->mat_registry.mat_default[IDX_MATERIAL_DEFAULT_GRAY_EMIT];
    a3real4Set(material_default_red->albedo.v, 1, 0, 0, 1);
    a3real4Set(material_default_yellow->albedo.v, 1, 1, 0, 1);
    a3real4Set(material_default_green->albedo.v, 0, 1, 0, 1);
    a3real4Set(material_default_cyan->albedo.v, 0, 1, 1, 1);
    a3real4Set(material_default_blue->albedo.v, 0, 0, 1, 1);
    a3real4Set(material_default_magenta->albedo.v, 1, 0, 1, 1);
    a3real4Set(material_default_white_emit->emissive.v, 1.0F, 1.0F, 1.0F, 1);
    a3real4Set(material_default_gray_emit->emissive.v, 0.5F, 0.5F, 0.5F, 1);

    // Room box shape:
    rtShapeBox* const box_inverted_room = &scene->shape_registry.box_inverted[IDX_SHAPE_BOX_INVERTED_ROOM];
    rtReal const room_size_2 = 0.5F * scene_object_base[IDX_MODEL_ROOM_BOX].scale.x;
    box_inverted_room->bounds_2[0].x = +room_size_2;
    box_inverted_room->bounds_2[0].y = +room_size_2;
    box_inverted_room->bounds_2[0].z = +room_size_2;
    box_inverted_room->bounds_2[0].w = 0.0F;
    box_inverted_room->bounds_2[1].x = -room_size_2;
    box_inverted_room->bounds_2[1].y = -room_size_2;
    box_inverted_room->bounds_2[1].z = -room_size_2;
    box_inverted_room->bounds_2[1].w = 0.0F;

    // Room box object: 
    rtObject* const obj_room = &scene->obj_registry.obj[UID_OBJECT_ROOM];
    a3mat4 const* const transform_room = &scene_model[IDX_MODEL_ROOM_BOX].modelViewMat;
    a3real const room_shift_z = -0.05F;
    scene->obj_registry.obj_box_inverted[num_objects_box_inverted++] = UID_OBJECT_ROOM;
    a3real4GetUnit(obj_room->normal.v, transform_room->v2.v);
    a3real4GetUnit(obj_room->tangent.v, transform_room->v0.v);
    a3real4GetUnit(obj_room->bitangent.v, transform_room->v1.v);
    a3real4Sum(obj_room->center.v, transform_room->v3.v, a3real4ProductS(tmp.v, obj_room->normal.v, room_shift_z));//< Shift down slightly
    obj_room->type_mat   = IDX_MATERIAL_TYPE_DEFAULT;
    obj_room->idx_mat    = IDX_MATERIAL_DEFAULT_GREEN;
    obj_room->type_shape = IDX_SHAPE_TYPE_BOX_INVERTED;
    obj_room->idx_shape  = IDX_SHAPE_BOX_INVERTED_ROOM;

    // Ground plane shape:
    rtShapePlane* const plane_finite_ground = &scene->shape_registry.plane_finite[IDX_SHAPE_PLANE_FINITE_GROUND];
    plane_finite_ground->size_2.x = room_size_2;
    plane_finite_ground->size_2.y = room_size_2;
    plane_finite_ground->size_2.z = 0.0F;
    plane_finite_ground->size_2.w = 0.0F;

    // Ground plane object:
    rtObject* const obj_ground = &scene->obj_registry.obj[UID_OBJECT_GROUND];
    a3real const ground_shift_z = -room_size_2 - room_shift_z;
    scene->obj_registry.obj_plane_finite[num_objects_plane_finite++] = UID_OBJECT_GROUND;
    obj_ground->normal    = obj_room->normal;
    obj_ground->tangent   = obj_room->tangent;
    obj_ground->bitangent = obj_room->bitangent;
    a3real4Sum(obj_ground->center.v, transform_room->v3.v, a3real4ProductS(tmp.v, obj_room->normal.v, ground_shift_z));//< Shift down to floor
    obj_ground->type_mat   = IDX_MATERIAL_TYPE_DEFAULT;
    obj_ground->idx_mat    = IDX_MATERIAL_DEFAULT_MAGENTA;
    obj_ground->type_shape = IDX_SHAPE_TYPE_PLANE_FINITE;
    obj_ground->idx_shape  = IDX_SHAPE_PLANE_FINITE_GROUND;

    // Lightbulb shape:
    rtShapeSphere* const sphere_lightbulb = &scene->shape_registry.sphere[IDX_SHAPE_SPHERE_LIGHTBULB];
    a3real const lightbulb_size = scene_object_base[IDX_MODEL_LIGHT_SPHERE].scale.x;
    rtScalarInit(&sphere_lightbulb->radius, lightbulb_size);

    // Lightbulb object:
    rtObject* const obj_lightbulb = &scene->obj_registry.obj[UID_OBJECT_LIGHTBULB];
    a3mat4 const* const transform_lightbulb = &scene_model[IDX_MODEL_LIGHT_SPHERE].modelViewMat;
    a3real const lightbulb_shift_z = -lightbulb_size + room_shift_z;
    scene->obj_registry.obj_sphere[num_objects_sphere++] = UID_OBJECT_LIGHTBULB;
    a3real4GetUnit(obj_lightbulb->normal.v, transform_lightbulb->v2.v);
    a3real4GetUnit(obj_lightbulb->tangent.v, transform_lightbulb->v0.v);
    a3real4GetUnit(obj_lightbulb->bitangent.v, transform_lightbulb->v1.v);
    a3real4Sum(obj_lightbulb->center.v, transform_lightbulb->v3.v, a3real4ProductS(tmp.v, obj_room->normal.v, lightbulb_shift_z));//< Shift down slightly
    obj_lightbulb->type_mat   = IDX_MATERIAL_TYPE_DEFAULT;
    obj_lightbulb->idx_mat    = IDX_MATERIAL_DEFAULT_WHITE_EMIT;
    obj_lightbulb->type_shape = IDX_SHAPE_TYPE_SPHERE;
    obj_lightbulb->idx_shape  = IDX_SHAPE_SPHERE_LIGHTBULB;

    // Sphere shapes:
    rtShapeSphere* const sphere_obj0 = &scene->shape_registry.sphere[IDX_SHAPE_SPHERE_OBJ0];
    a3real const sphere0_size = scene_object_base[IDX_MODEL_SPHERE0].scale.x;
    rtScalarInit(&sphere_obj0->radius, sphere0_size);

    rtShapeSphere* const sphere_obj1 = &scene->shape_registry.sphere[IDX_SHAPE_SPHERE_OBJ1];
    a3real const sphere1_size = scene_object_base[IDX_MODEL_SPHERE1].scale.x;
    rtScalarInit(&sphere_obj1->radius, sphere1_size);

    // Sphere objects:
    rtObject* const obj_sphere0 = &scene->obj_registry.obj[UID_OBJECT_SPHERE0];
    a3mat4 const* const transform_sphere0 = &scene_model[IDX_MODEL_SPHERE0].modelViewMat;
    scene->obj_registry.obj_sphere[num_objects_sphere++] = UID_OBJECT_SPHERE0;
    obj_sphere0->center = transform_sphere0->v3;//< Center
    a3real4GetUnit(obj_sphere0->normal.v, transform_sphere0->v2.v);
    a3real4GetUnit(obj_sphere0->tangent.v, transform_sphere0->v0.v);
    a3real4GetUnit(obj_sphere0->bitangent.v, transform_sphere0->v1.v);
    obj_sphere0->type_mat   = IDX_MATERIAL_TYPE_DEFAULT;
    obj_sphere0->idx_mat    = IDX_MATERIAL_DEFAULT_RED;
    obj_sphere0->type_shape = IDX_SHAPE_TYPE_SPHERE;
    obj_sphere0->idx_shape  = IDX_SHAPE_SPHERE_OBJ0;

    rtObject* const obj_sphere1 = &scene->obj_registry.obj[UID_OBJECT_SPHERE1];
    a3mat4 const* const transform_sphere1 = &scene_model[IDX_MODEL_SPHERE1].modelViewMat;
    scene->obj_registry.obj_sphere[num_objects_sphere++] = UID_OBJECT_SPHERE1;
    obj_sphere1->center = transform_sphere1->v3;//< Center
    a3real4GetUnit(obj_sphere1->normal.v, transform_sphere1->v2.v);
    a3real4GetUnit(obj_sphere1->tangent.v, transform_sphere1->v0.v);
    a3real4GetUnit(obj_sphere1->bitangent.v, transform_sphere1->v1.v);
    obj_sphere1->type_mat   = IDX_MATERIAL_TYPE_DEFAULT;
    obj_sphere1->idx_mat    = IDX_MATERIAL_DEFAULT_YELLOW;
    obj_sphere1->type_shape = IDX_SHAPE_TYPE_SPHERE;
    obj_sphere1->idx_shape  = IDX_SHAPE_SPHERE_OBJ1;

    // Box shapes:
    rtShapeBox* const box_obj0 = &scene->shape_registry.box[IDX_SHAPE_BOX_OBJ0];
    a3real const box0_size_2 = 0.5F * scene_object_base[IDX_MODEL_BOX0].scale.x;
    box_obj0->bounds_2[0].x = +box0_size_2;
    box_obj0->bounds_2[0].y = +box0_size_2;
    box_obj0->bounds_2[0].z = +box0_size_2;
    box_obj0->bounds_2[0].w = 0.0F;
    box_obj0->bounds_2[1].x = -box0_size_2;
    box_obj0->bounds_2[1].y = -box0_size_2;
    box_obj0->bounds_2[1].z = -box0_size_2;
    box_obj0->bounds_2[1].w = 0.0F;

    rtShapeBox* const box_obj1 = &scene->shape_registry.box[IDX_SHAPE_BOX_OBJ1];
    a3real const box1_size_2 = 0.5F * scene_object_base[IDX_MODEL_BOX1].scale.x;
    box_obj1->bounds_2[0].x = +box1_size_2;
    box_obj1->bounds_2[0].y = +box1_size_2;
    box_obj1->bounds_2[0].z = +box1_size_2;
    box_obj1->bounds_2[0].w = 0.0F;
    box_obj1->bounds_2[1].x = -box1_size_2;
    box_obj1->bounds_2[1].y = -box1_size_2;
    box_obj1->bounds_2[1].z = -box1_size_2;
    box_obj1->bounds_2[1].w = 0.0F;

    // Box objects:
    rtObject* const obj_box0 = &scene->obj_registry.obj[UID_OBJECT_BOX0];
    a3mat4 const* const transform_box0 = &scene_model[IDX_MODEL_BOX0].modelViewMat;
    scene->obj_registry.obj_box[num_objects_box++] = UID_OBJECT_BOX0;
    obj_box0->center = transform_box0->v3;//< Center
    a3real4GetUnit(obj_box0->normal.v, transform_box0->v2.v);
    a3real4GetUnit(obj_box0->tangent.v, transform_box0->v0.v);
    a3real4GetUnit(obj_box0->bitangent.v, transform_box0->v1.v);
    obj_box0->type_mat   = IDX_MATERIAL_TYPE_DEFAULT;
    obj_box0->idx_mat    = IDX_MATERIAL_DEFAULT_CYAN;
    obj_box0->type_shape = IDX_SHAPE_TYPE_BOX;
    obj_box0->idx_shape  = IDX_SHAPE_BOX_OBJ0;

    rtObject* const obj_box1 = &scene->obj_registry.obj[UID_OBJECT_BOX1];
    a3mat4 const* const transform_box1 = &scene_model[IDX_MODEL_BOX1].modelViewMat;
    scene->obj_registry.obj_box[num_objects_box++] = UID_OBJECT_BOX1;
    obj_box1->center = transform_box1->v3;//< Center
    a3real4GetUnit(obj_box1->normal.v, transform_box1->v2.v);
    a3real4GetUnit(obj_box1->tangent.v, transform_box1->v0.v);
    a3real4GetUnit(obj_box1->bitangent.v, transform_box1->v1.v);
    obj_box1->type_mat   = IDX_MATERIAL_TYPE_DEFAULT;
    obj_box1->idx_mat    = IDX_MATERIAL_DEFAULT_BLUE;
    obj_box1->type_shape = IDX_SHAPE_TYPE_BOX;
    obj_box1->idx_shape  = IDX_SHAPE_BOX_OBJ1;
}


// sub-routine for rendering the demo state using the shading pipeline
void a3rendering_render(a3_DemoState const* demoState, a3_Scene_Rendering const* scene, a3f64 const dt)
{
	// pointers
	const a3_VertexDrawable* currentDrawable;
	const a3_SceneShaderProgram* currentDemoProgram;

	// framebuffers
	const a3_Framebuffer* currentWriteFBO;
	const a3_Framebuffer* currentReadFBO, * currentDisplayFBO;

	// indices
	a3ui32 i, j;

	// RGB
	const a3vec4 rgba4[] = {
		{ 1.00f, 0.00f, 0.00f, 1.00f },	// red
		{ 1.00f, 0.25f, 0.00f, 1.00f },
		{ 1.00f, 0.50f, 0.00f, 1.00f },	// orange
		{ 1.00f, 0.75f, 0.00f, 1.00f },
		{ 1.00f, 1.00f, 0.00f, 1.00f },	// yellow
		{ 0.75f, 1.00f, 0.00f, 1.00f },
		{ 0.50f, 1.00f, 0.00f, 1.00f },	// lime
		{ 0.25f, 1.00f, 0.00f, 1.00f },
		{ 0.00f, 1.00f, 0.00f, 1.00f },	// green
		{ 0.00f, 1.00f, 0.25f, 1.00f },
		{ 0.00f, 1.00f, 0.50f, 1.00f },	// aqua
		{ 0.00f, 1.00f, 0.75f, 1.00f },
		{ 0.00f, 1.00f, 1.00f, 1.00f },	// cyan
		{ 0.00f, 0.75f, 1.00f, 1.00f },
		{ 0.00f, 0.50f, 1.00f, 1.00f },	// sky
		{ 0.00f, 0.25f, 1.00f, 1.00f },
		{ 0.00f, 0.00f, 1.00f, 1.00f },	// blue
		{ 0.25f, 0.00f, 1.00f, 1.00f },
		{ 0.50f, 0.00f, 1.00f, 1.00f },	// purple
		{ 0.75f, 0.00f, 1.00f, 1.00f },
		{ 1.00f, 0.00f, 1.00f, 1.00f },	// magenta
		{ 1.00f, 0.00f, 0.75f, 1.00f },
		{ 1.00f, 0.00f, 0.50f, 1.00f },	// rose
		{ 1.00f, 0.00f, 0.25f, 1.00f },
	};
	const a3vec4 grey4[] = {
		{ 0.5f, 0.5f, 0.5f, 1.0f },	// solid grey
		{ 0.5f, 0.5f, 0.5f, 0.5f },	// translucent grey
	};
	const a3real
		* const red = rgba4[0].v, * const orange = rgba4[2].v, * const yellow = rgba4[4].v, * const lime = rgba4[6].v,
		* const green = rgba4[8].v, * const aqua = rgba4[10].v, * const cyan = rgba4[12].v, * const sky = rgba4[14].v,
		* const blue = rgba4[16].v, * const purple = rgba4[18].v, * const magenta = rgba4[20].v, * const rose = rgba4[22].v,
		* const grey = grey4[0].v, * const grey_t = grey4[1].v;
	const a3ui32 hueCount = sizeof(rgba4) / sizeof(*rgba4);

	// camera used for drawing
	const a3_SceneProjector* activeCamera = scene->projector + scene->activeCamera;
	const a3_SceneObject* activeCameraObject = activeCamera->sceneObject;

	// current hull for scene object being rendered, for convenience
	const a3_SceneObject* currentSceneObject, * endSceneObject;

	// temp drawable pointers
	const a3_VertexDrawable* drawable[] = {
		0,
		demoState->draw_node,
		demoState->draw_node,
		demoState->draw_unit_box,		// skybox

		demoState->draw_node,			// room root
		demoState->draw_unit_box,       // boxes
		demoState->draw_unit_box,       // 
		demoState->draw_unit_sphere,    // spheres
		demoState->draw_unit_sphere,    // 
		demoState->draw_unit_sphere,    // light
		demoState->draw_unit_box,       // room
	};

	// temp texture pointers
	const a3_Texture* texture_dm[] = {
		0,
		0,
		0,
		demoState->tex_checker,			// skybox
		
		0,                  			// room root
		demoState->tex_checker,			// boxes
		demoState->tex_checker,			// 
		demoState->tex_checker,			// spheres
		demoState->tex_checker,			// 
		demoState->tex_checker,			// light
		demoState->tex_checker,			// room
	};

    // model inversion
    const a3boolean invert_model[] = {
        0,
        0,
        0,
        1,

        0,
        0,
        0,
        0,
        0,
        0,
        1,
    };

	// forward pipeline shader programs
	const a3_SceneShaderProgram* renderProgram[rendering_pipeline_max][rendering_render_max] = {
		{
			demoState->prog_drawColorUnif,
			demoState->prog_drawTexture,
			demoState->prog_drawLambert,
			demoState->prog_drawPhong,
            demoState->prog_drawRT,
		},
	};

	// display shader programs
	const a3_SceneShaderProgram* displayProgram[rendering_display_max] = {
		demoState->prog_drawTexture,
	};

	// framebuffers to which to write based on pipeline mode
	const a3_Framebuffer* writeFBO[rendering_pass_max] = {
		demoState->fbo_scene_c16d24s8_mrt,
		demoState->fbo_composite_c16,
	};

	// framebuffers from which to read based on pipeline mode
	const a3_Framebuffer* readFBO[rendering_pass_max][4] = {
		{ 0, },
		{ demoState->fbo_scene_c16d24s8_mrt, },
	};

	// target info
	a3_Scene_Rendering_RenderProgramName const render = scene->render;
	a3_Scene_Rendering_DisplayProgramName const display = scene->display;
	a3_Scene_Rendering_PipelineName const pipeline = scene->pipeline;
	a3_Scene_Rendering_PassName const pass = scene->pass;
	a3_Scene_Rendering_TargetName const targetIndex = scene->targetIndex[pass], targetCount = scene->targetCount[pass];
	a3_Scene_Rendering_PassName currentPass;

	// FSQ matrix
	const a3mat4 fsq = {
		2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 2.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	// bias matrix
	const a3mat4 bias = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f,
	};
	const a3mat4 unbias = {
		 2.0f,  0.0f,  0.0f, 0.0f,
		 0.0f,  2.0f,  0.0f, 0.0f,
		 0.0f,  0.0f,  2.0f, 0.0f,
		-1.0f, -1.0f, -1.0f, 1.0f,
	};

	// final model matrix and full matrix stack
	a3mat4 projectionMat = activeCamera->projectionMat;
	a3mat4 projectionMatInv = activeCamera->projectionMatInv;
	a3mat4 viewMat = scene->sceneGraphState->objectSpaceInv->hpose_base[activeCameraObject->sceneGraphIndex].transformMat;
	a3mat4 viewProjectionMat;
	a3mat4 projectionBiasMat, projectionBiasMat_inv;
	a3mat4 modelMat, modelViewMat, modelViewProjectionMat;
    a3vec4 pixelSizeAndInv;

    a3_SceneObject const* const scene_object_base = scene->obj_room_box;
    a3_SceneModelMatrixStack  const* const room_model_base  = &scene->modelMatrixStack[scene_object_base - scene->object_scene];
    a3_SceneViewerMatrixStack const* const room_viewer_base = &scene->viewerMatrixStack[scene->proj_camera_main - scene->projector];

    // RAY-TRACING SCENE
    rtScene scene_raytracing;
    rtBuildScene(&scene_raytracing, scene_object_base, room_model_base);

	// init
	a3real4x4Product(viewProjectionMat.m, projectionMat.m, viewMat.m);
	a3real4x4Product(projectionBiasMat.m, bias.m, projectionMat.m);
	a3real4x4Product(projectionBiasMat_inv.m, projectionMatInv.m, unbias.m);
	

	//-------------------------------------------------------------------------
	// 0) PRE-SCENE PASS: shadow pass renders scene to depth-only
	//	- activate shadow pass framebuffer
	//	- draw scene
	//		- clear depth buffer
	//		- render shapes using appropriate shaders
	//		- capture depth


	//-------------------------------------------------------------------------
	// 1) SCENE PASS: render scene with desired shader
	//	- activate scene framebuffer
	//	- draw scene
	//		- clear buffers
	//		- render shapes using appropriate shaders
	//		- capture color and depth

	// select target framebuffer
	currentPass = rendering_passScene;
	currentWriteFBO = writeFBO[currentPass];
	switch (pipeline)
	{
		// shading with MRT
	case rendering_forward:
		// target scene framebuffer
		a3scene_setSceneState(currentWriteFBO, demoState->displaySkybox);
		break;
	}


	// optional stencil test before drawing objects
	//a3real4x4SetScale(modelMat.m, a3real_four);
	//if (demoState->stencilTest)
	//	a3scene_drawStencilTest(modelViewProjectionMat.m, viewProjectionMat.m, modelMat.m, demoState->prog_drawColorUnif, demoState->draw_unit_sphere);


	// select program based on settings
	currentDemoProgram = renderProgram[pipeline][render];
	a3shaderProgramActivate(currentDemoProgram->program);

	// send shared data: 
	//	- projection matrix
	//	- light data
	//	- activate shared textures including atlases if using
	//	- shared animation data
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uP, 1, projectionMat.mm);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uP_inv, 1, projectionMatInv.mm);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uPB, 1, projectionBiasMat.mm);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uPB_inv, 1, projectionBiasMat_inv.mm);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uAtlas, 1, a3mat4_identity.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, hueCount, rgba4->v);
    if (demoState->updateAnimation)
        a3shaderUniformSendDouble(a3unif_single, currentDemoProgram->uTime, 1, &demoState->timer_display->totalTime);
    
    // send target dimensions
    pixelSizeAndInv.x = (a3f32)currentWriteFBO->frameWidth;
    pixelSizeAndInv.y = (a3f32)currentWriteFBO->frameHeight;
    pixelSizeAndInv.z = 1.0f / pixelSizeAndInv.x;
    pixelSizeAndInv.w = 1.0f / pixelSizeAndInv.y;
    a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uAxis, 1, pixelSizeAndInv.v);

	// select pipeline algorithm
	glDisable(GL_BLEND);
	switch (pipeline)
	{
		// scene pass using forward pipeline
	case rendering_forward: {

		// forward shading algorithms
		switch (scene->render)
		{
		case rendering_renderSolid:
		case rendering_renderTexture:
			// individual object requirements: 
			//	- modelviewprojection
			//	- modelview
			//	- modelview for normals
			//	- per-object animation data
			for (currentSceneObject = scene->obj_room_box, endSceneObject = scene->obj_room_enclosure,
				j = (a3ui32)(currentSceneObject - scene->object_scene);
				currentSceneObject <= endSceneObject;
				++j, ++currentSceneObject)
			{
				// send data and draw
				i = (j * 2 + 11) % hueCount;
				currentDrawable = drawable[currentSceneObject - scene->obj_world_root];
				a3textureActivate(texture_dm[j], a3tex_unit00);
				a3real4x4Product(modelViewProjectionMat.m, viewProjectionMat.m, currentSceneObject->modelMat.m);
				a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
				a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, rgba4[i].v);
				a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uIndex, 1, &j);
                if (invert_model[j])
                {
                    glCullFace(GL_FRONT);
                    a3vertexDrawableActivateAndRender(currentDrawable);
                    glCullFace(GL_BACK);
                }
                else
                {
                    a3vertexDrawableActivateAndRender(currentDrawable);
                }
			}
			break;
		case rendering_renderLambert:
		case rendering_renderPhong:
			for (currentSceneObject = scene->obj_room_box, endSceneObject = scene->obj_room_enclosure,
				j = (a3ui32)(currentSceneObject - scene->object_scene);
				currentSceneObject <= endSceneObject;
				++j, ++currentSceneObject)
			{
				// send data and draw
				i = (j * 2 + 11) % hueCount;
				currentDrawable = drawable[currentSceneObject - scene->obj_world_root];
				a3textureActivate(texture_dm[j], a3tex_unit00);
				a3textureActivate(texture_dm[j], a3tex_unit01);
				a3real4x4Product(modelViewMat.m, activeCameraObject->modelMatInv.m, currentSceneObject->modelMat.m);
				a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMV, 1, modelViewMat.mm);
				a3scene_quickInvertTranspose_internal(modelViewMat.m);
				modelViewMat.v3 = a3vec4_zero;
				a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMV_nrm, 1, modelViewMat.mm);
				a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, rgba4[i].v);
				a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uIndex, 1, &j);
                if (invert_model[j])
                {
                    glCullFace(GL_FRONT);
                    a3vertexDrawableActivateAndRender(currentDrawable);
                    glCullFace(GL_BACK);
                }
                else
                {
                    a3vertexDrawableActivateAndRender(currentDrawable);
                }
			}
			break;
        case rendering_renderRT: {
            rtBuildHierarchy();
            a3demo_uploadTransformStacks(demoState->ubo_transformStack,
                room_model_base, (a3ui32)(scene->obj_room_enclosure - scene->obj_room),
                room_viewer_base, 1,
                &scene_raytracing);
            a3shaderUniformBufferActivate(demoState->ubo_transformStack, 0);
            for (currentSceneObject = scene->obj_room_enclosure, endSceneObject = scene->obj_room_enclosure,
                j = (a3ui32)(currentSceneObject - scene->object_scene);
                currentSceneObject <= endSceneObject;
                ++j, ++currentSceneObject)
            {
                a3mat4 modelViewMat_tmp;
                a3mat4 modelViewMat_scale;
                a3real4x4SetScale(modelViewMat_scale.m, 1.05F);

                // send data and draw
                i = (j * 2 + 11) % hueCount;
                currentDrawable = drawable[currentSceneObject - scene->obj_world_root];
                a3textureActivate(texture_dm[j], a3tex_unit00);
                a3textureActivate(texture_dm[j], a3tex_unit01);
                a3real4x4Product(modelViewMat_tmp.m, activeCameraObject->modelMatInv.m, currentSceneObject->modelMat.m);
                a3real4x4Product(modelViewMat.m, modelViewMat_tmp.m, modelViewMat_scale.m);
                a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMV, 1, modelViewMat.mm);
                a3scene_quickInvertTranspose_internal(modelViewMat.m);
                modelViewMat.v3 = a3vec4_zero;
                a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMV_nrm, 1, modelViewMat.mm);
                a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, rgba4[i].v);
                a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uIndex, 1, &j);
                if (invert_model[j])
                {
                    glCullFace(GL_FRONT);
                    a3vertexDrawableActivateAndRender(currentDrawable);
                    glCullFace(GL_BACK);
                }
                else
                {
                    a3vertexDrawableActivateAndRender(currentDrawable);
                }
            }
        }	break;
		}
	}	break;
		// end forward scene pass
	}


	// stop using stencil
	if (demoState->stencilTest)
		glDisable(GL_STENCIL_TEST);


	//-------------------------------------------------------------------------
	// COMPOSITE PASS
	//	- activate composite framebuffer
	//	- composite scene layers

	currentPass = rendering_passComposite;
	currentWriteFBO = writeFBO[currentPass];
	a3framebufferActivate(currentWriteFBO);

	// composite skybox
	currentDemoProgram = demoState->displaySkybox ? demoState->prog_drawTexture : demoState->prog_drawColorUnif;
	modelMat = scene->sceneGraphState->objectSpace->hpose_base[scene->obj_skybox->sceneGraphIndex].transformMat;
	a3scene_drawModelTexturedColored_invertModel(modelViewProjectionMat.m, viewProjectionMat.m, modelMat.m, a3mat4_identity.m, currentDemoProgram, demoState->draw_unit_box, demoState->tex_skybox_clouds, a3vec4_one.v);
	a3scene_enableCompositeBlending();

	// draw textured quad with previous pass image on it
	// repeat as necessary to complete composite
	currentDrawable = demoState->draw_unit_plane_z;
	a3vertexDrawableActivate(currentDrawable);

	switch (pipeline)
	{
	case rendering_forward:
		// use simple texturing program
		currentDemoProgram = demoState->prog_drawTexture;
		a3shaderProgramActivate(currentDemoProgram->program);
		// scene (color)
		currentReadFBO = readFBO[currentPass][0];
		a3framebufferBindColorTexture(currentReadFBO, a3tex_unit00, 0);
		break;
	}
	// reset other uniforms
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, fsq.mm);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uAtlas, 1, a3mat4_identity.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, a3vec4_one.v);
	a3vertexDrawableRenderActive();


	//-------------------------------------------------------------------------
	// PREPARE FOR POST-PROCESSING
	//	- double buffer swap (if applicable)
	//	- ensure blending is disabled
	//	- re-activate FSQ drawable IF NEEDED (i.e. changed in previous step)
	glDisable(GL_BLEND);
	currentDrawable = demoState->draw_unit_plane_z;
	a3vertexDrawableActivate(currentDrawable);


	//-------------------------------------------------------------------------
	// POST-PROCESSING
	//	- activate target framebuffer
	//	- activate texture from previous framebuffer
	//	- draw FSQ with processing program active


	//-------------------------------------------------------------------------
	// DISPLAY: final pass, perform and present final composite
	//	- finally draw to back buffer
	//	- select display texture(s)
	//	- activate final pass program
	//	- draw final FSQ

	// revert to back buffer and disable depth testing
	a3framebufferDeactivateSetViewport(a3fbo_depthDisable,
		-demoState->frameBorder, -demoState->frameBorder, demoState->frameWidth, demoState->frameHeight);

	// select framebuffer to display based on mode
	currentDisplayFBO = writeFBO[scene->pass];

	// select output to display
	switch (scene->pass)
	{
	case rendering_passScene:
		if (currentDisplayFBO->color && (!currentDisplayFBO->depthStencil || targetIndex < targetCount - 1))
			a3framebufferBindColorTexture(currentDisplayFBO, a3tex_unit00, targetIndex);
		else
			a3framebufferBindDepthTexture(currentDisplayFBO, a3tex_unit00);
		break;
	case rendering_passComposite:
		a3framebufferBindColorTexture(currentDisplayFBO, a3tex_unit00, targetIndex);
		break;
	}


	// final display: activate desired final program and draw FSQ
	if (currentDisplayFBO)
	{
		// prepare for final draw
		currentDrawable = demoState->draw_unit_plane_z;
		a3vertexDrawableActivate(currentDrawable);

		// determine if additional passes are required
		currentDemoProgram = displayProgram[display];
		a3shaderProgramActivate(currentDemoProgram->program);

		switch (scene->display)
		{
			// most basic option: simply display texture
		case rendering_displayTexture:
			break;
		}

		// done
		a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, fsq.mm);
		a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uAtlas, 1, a3mat4_identity.mm);
		a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, a3vec4_one.v);
		a3vertexDrawableRenderActive();
	}


	//-------------------------------------------------------------------------
	// OVERLAYS: done after FSQ so they appear over everything else
	//	- disable depth testing
	//	- draw overlays appropriately

	// enable alpha
	a3scene_enableCompositeBlending();

	// scene overlays
	if (scene->pass >= rendering_passScene)
	{
		if (demoState->displayGrid || demoState->displayTangentBases || demoState->displayWireframe)
		{
			// activate scene FBO and clear color; reuse depth
			currentWriteFBO = demoState->fbo_scene_c16d24s8_mrt;
			a3framebufferActivate(currentWriteFBO);
			glDisable(GL_STENCIL_TEST);
			glClear(GL_COLOR_BUFFER_BIT);
		
			// draw grid aligned to world
			if (demoState->displayGrid)
			{
				a3scene_drawModelSolidColor(modelViewProjectionMat.m, viewProjectionMat.m, a3mat4_identity.m, demoState->prog_drawColorUnif, demoState->draw_grid, blue);
			}
		
			if (demoState->displayTangentBases || demoState->displayWireframe)
			{
				const a3i32 flag[1] = { demoState->displayTangentBases * 3 + demoState->displayWireframe * 4 };
				const a3f32 size[1] = { 0.015625f };

				currentDemoProgram = demoState->prog_drawTangentBasis;
				a3shaderProgramActivate(currentDemoProgram->program);

				// projection matrix
				a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uP, 1, projectionMat.mm);
				// wireframe color
				a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor0, hueCount, rgba4->v);
				// blend color
				a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, a3vec4_one.v);
				// tangent basis size
				a3shaderUniformSendFloat(a3unif_single, currentDemoProgram->uSize, 1, size);
				// overlay flag
				a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uFlag, 1, flag);
                
				// draw objects again
				for (currentSceneObject = scene->obj_room_box, endSceneObject = scene->obj_room_enclosure,
					j = (a3ui32)(currentSceneObject - scene->object_scene);
					currentSceneObject <= endSceneObject;
					++j, ++currentSceneObject)
				{
					// calculate per-object uniforms
					i = (j * 2 + 23) % hueCount;
					currentDrawable = drawable[currentSceneObject - scene->obj_world_root];
					a3real4x4Product(modelViewMat.m, activeCameraObject->modelMatInv.m, currentSceneObject->modelMat.m);
					a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMV, 1, modelViewMat.mm);
					a3scene_quickInvertTranspose_internal(modelViewMat.m);
					modelViewMat.v3 = a3vec4_zero;
					a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMV_nrm, 1, modelViewMat.mm);
					a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uAtlas, 1, a3mat4_identity.mm);
					a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uIndex, 1, &i);
					a3vertexDrawableActivateAndRender(currentDrawable);
				}
			}

			// display color target with scene overlays
			a3framebufferDeactivateSetViewport(a3fbo_depthDisable,
				-demoState->frameBorder, -demoState->frameBorder, demoState->frameWidth, demoState->frameHeight);
			currentDrawable = demoState->draw_unit_plane_z;
			currentDemoProgram = demoState->prog_drawTexture;
			a3vertexDrawableActivate(currentDrawable);
			a3shaderProgramActivate(currentDemoProgram->program);
			a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, fsq.mm);
			a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uAtlas, 1, a3mat4_identity.mm);
			a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, a3vec4_one.v);
			a3framebufferBindColorTexture(currentWriteFBO, a3tex_unit00, 0);
			a3vertexDrawableRenderActive();
		}

		// hidden volumes
		if (demoState->displayHiddenVolumes)
		{
            // ray fired
            if (scene->test_ray_fired)
            {
                a3_SceneProjector const* projector = scene->projector + scene->activeCamera;
                a3real const* const color = scene->test_ray_hit ? green : red;
                a3real const param = scene->test_ray_hit ? scene->test_ray_param : a3real_four;

                modelMat = projector->sceneObject->modelMat;
                a3real3MulS(modelMat.v0.v, (a3real)(0.025));
                a3real3MulS(modelMat.v1.v, (a3real)(0.025));
                a3real3MulS(modelMat.v2.v, (a3real)(0.025));
                a3rayComputePos(modelMat.v3.v, scene->test_ray.p_origin.v, scene->test_ray.v_direction.v, a3real_zero);
                a3scene_drawModelSolidColor(modelViewProjectionMat.m, viewProjectionMat.m, modelMat.m, demoState->prog_drawColorUnif, demoState->draw_unit_sphere, blue);
                a3rayComputePos(modelMat.v3.v, scene->test_ray.p_origin.v, scene->test_ray.v_direction.v, param);
                a3scene_drawModelSolidColor(modelViewProjectionMat.m, viewProjectionMat.m, modelMat.m, demoState->prog_drawColorUnif, demoState->draw_unit_sphere, color);
            }
		}


		// superimpose axes
		// draw coordinate axes in front of everything
		currentDemoProgram = demoState->prog_drawColorAttrib;
		a3shaderProgramActivate(currentDemoProgram->program);
		a3vertexDrawableActivate(demoState->draw_axes);

		// center of world from current viewer
		// also draw other viewer/viewer-like object in scene
		if (demoState->displayWorldAxes)
		{
			a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, viewProjectionMat.mm);
			a3vertexDrawableRenderActive();
		}

		// individual objects (based on scene graph)
		if (demoState->displayObjectAxes)
		{
			for (currentSceneObject = scene->obj_room_box, endSceneObject = scene->obj_room_enclosure;
				currentSceneObject <= endSceneObject;
				++currentSceneObject)
			{
				j = (a3ui32)(currentSceneObject - scene->object_scene);
				modelMat = scene->sceneGraphState->objectSpace->hpose_base[currentSceneObject->sceneGraphIndex].transformMat;
				a3scene_drawModelSimple(modelViewProjectionMat.m, viewProjectionMat.m, modelMat.m, currentDemoProgram);
			}
		}
	}
}


//-----------------------------------------------------------------------------
