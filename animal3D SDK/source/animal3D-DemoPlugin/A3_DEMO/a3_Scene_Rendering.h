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

	a3_Scene_Rendering.h
	Demo mode interface: rendering scene.

	********************************************
	*** THIS IS ONE DEMO MODE'S HEADER FILE  ***
	********************************************
*/

#ifndef __ANIMAL3D_SCENE_RENDERING_H
#define __ANIMAL3D_SCENE_RENDERING_H


//-----------------------------------------------------------------------------

#include "_a3_scene_utilities/a3_SceneObject.h"

#include "_animation/a3_HierarchyState.h"

#include "_physics/a3_Ray.h"


//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif	// __cplusplus


//-----------------------------------------------------------------------------

	// maximum unique objects
	enum
	{
		renderingMaxCount_sceneObject = 24,
		renderingMaxCount_projector = 1,
	};

	// scene object rendering program names
	typedef enum a3_Scene_Rendering_RenderProgramName
	{
		rendering_renderSolid,			// solid color
		rendering_renderTexture,		// textured
		rendering_renderLambert,		// Lambert shading model
		rendering_renderPhong,			// Phong shading model
        //rendering_renderRT,             // ray-tracing
        rendering_renderPhongStereo,    // Phong shading model stereoscopic

		rendering_render_max
	} a3_Scene_Rendering_RenderProgramName;

	// final display modes
	typedef enum a3_Scene_Rendering_DisplayProgramName
	{
		rendering_displayTexture,			// display simple texture
        rendering_displayStereo,            // display texture with stereo processing

		rendering_display_max
	} a3_Scene_Rendering_DisplayProgramName;

	// active camera names
	typedef enum a3_Scene_Rendering_ActiveCameraName
	{
		rendering_cameraSceneViewer,		// scene viewing camera

		rendering_camera_max
	} a3_Scene_Rendering_ActiveCameraName;

	// pipeline names
	typedef enum a3_Scene_Rendering_PipelineName
	{
		rendering_forward,				// forward lighting pipeline

		rendering_pipeline_max
	} a3_Scene_Rendering_PipelineName;

	// render passes
    typedef enum a3_Scene_Rendering_PassName
    {
        rendering_passScene,				// render scene objects
        rendering_passComposite,			// composite layers

		rendering_pass_max
	} a3_Scene_Rendering_PassName;

	// render target names
	typedef enum a3_Scene_Rendering_TargetName
	{
		rendering_scene_finalcolor = 0,	// final display color
		rendering_scene_fragdepth,		// fragment depth
	
		rendering_target_scene_max
	} a3_Scene_Rendering_TargetName;

	// control targets
	typedef enum a3_Scene_Rendering_ControlTarget
	{
		rendering_ctrl_camera,

		rendering_ctrlmode_max
	} a3_Scene_Rendering_ControlTarget;
	

//-----------------------------------------------------------------------------

	// demo mode for basic shading
	typedef struct a3_Scene_Rendering
	{
		a3_Scene_Rendering_RenderProgramName render;
		a3_Scene_Rendering_DisplayProgramName display;
		a3_Scene_Rendering_ActiveCameraName activeCamera;

		a3_Scene_Rendering_PipelineName pipeline;
		a3_Scene_Rendering_PassName pass;
		a3_Scene_Rendering_TargetName targetIndex[rendering_pass_max], targetCount[rendering_pass_max];

		// scene graph
		a3_Hierarchy sceneGraph[1];
		a3_HierarchyState sceneGraphState[1];
		a3_SceneModelMatrixStack modelMatrixStack[renderingMaxCount_sceneObject];
        a3_SceneViewerMatrixStack viewerMatrixStack[renderingMaxCount_projector];

		// control modes
		a3_Scene_Rendering_ControlTarget ctrl_target;

        //// test ray and hull
        //a3_Hull   test_hull;
        //a3_Ray    test_ray;
        //a3real    test_ray_param;
        //a3boolean test_ray_fired;
        //a3boolean test_ray_hit;

        // basic animation
        a3real rotate_time;


		// objects
		union {
			a3_SceneObject object_scene[renderingMaxCount_sceneObject];
			struct {
				a3_SceneObject
					obj_world_root[1];
				a3_SceneObject
					obj_camera_main[1];
				a3_SceneObject
					obj_light_main[1];
				a3_SceneObject
					obj_skybox[1];
				
				a3_SceneObject
					obj_room[1];
				a3_SceneObject
					obj_room_box[2],
                    obj_room_sphere[2],
                    obj_room_lightbulb[1],
                    obj_room_enclosure[1];

                a3_SceneObject
                    obj_material_container[1];
                a3_SceneObject
                    obj_material_ball[3];
			};
		};
		union {
			a3_SceneProjector projector[renderingMaxCount_projector];
			struct {
				a3_SceneProjector
					proj_camera_main[1];
			};
		};
	} a3_Scene_Rendering;


//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !__ANIMAL3D_SCENE_RENDERING_H