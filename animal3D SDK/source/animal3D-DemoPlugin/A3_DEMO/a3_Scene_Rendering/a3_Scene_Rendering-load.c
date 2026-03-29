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

	a3_Scene_Rendering-load.c/.cpp
	Demo mode implementations: rendering scene.

	********************************************
	*** LOADING FOR RENDERING SCENE MODE     ***
	********************************************
*/

//-----------------------------------------------------------------------------

#include "../a3_Scene_Rendering.h"

#include "../a3_DemoState.h"


//-----------------------------------------------------------------------------

// utility to load scene
void a3rendering_init_scene(a3_DemoState const* demoState, a3_Scene_Rendering* scene)
{
    a3_FileStream fileStream[1] = { 0 };
    const a3byte* const renderingStream = "./data/gpro26_base_render_active_2.dat";
    const a3boolean force_disable_streaming = true;


    // stream animation assets
    const a3boolean streaming = demoState->streaming & !force_disable_streaming;
    if (streaming && a3fileStreamOpenRead(fileStream, renderingStream))
    {
        // load scene graph
        a3hierarchyLoadBinary(scene->sceneGraph, fileStream);

        // done
        a3fileStreamClose(fileStream);
    }
    // not streaming or stream doesn't exist
    else if (a3fileStreamOpenWrite(fileStream, renderingStream))
    {
        // set up scenegraph
        a3hierarchyCreate(scene->sceneGraph, renderingMaxCount_sceneObject, 0);
        a3hierarchySetNode(scene->sceneGraph,  0, -1, "scene_world_root");
        a3hierarchySetNode(scene->sceneGraph,  1,  0, "scene_camera_main");
        a3hierarchySetNode(scene->sceneGraph,  2,  0, "scene_light_main");
        a3hierarchySetNode(scene->sceneGraph,  3,  0, "scene_skybox");

        a3hierarchySetNode(scene->sceneGraph,  4,  0, "scene_room");
        a3hierarchySetNode(scene->sceneGraph,  5,  4, "scene_room_box_0");
        a3hierarchySetNode(scene->sceneGraph,  6,  4, "scene_room_box_1");
        a3hierarchySetNode(scene->sceneGraph,  7,  4, "scene_room_sphere_0");
        a3hierarchySetNode(scene->sceneGraph,  8,  4, "scene_room_sphere_1");
        a3hierarchySetNode(scene->sceneGraph,  9,  4, "scene_room_lightbulb");
        a3hierarchySetNode(scene->sceneGraph, 10,  4, "scene_room_enclosure");

        a3hierarchySetNode(scene->sceneGraph, 11,  0, "scene_material_container");
        a3hierarchySetNode(scene->sceneGraph, 12, 11, "scene_material_ball_0");
        a3hierarchySetNode(scene->sceneGraph, 13, 11, "scene_material_ball_1");
        a3hierarchySetNode(scene->sceneGraph, 14, 11, "scene_material_ball_2");

        // save scene graph
        a3hierarchySaveBinary(scene->sceneGraph, fileStream);

        // done
        a3fileStreamClose(fileStream);
    }


    // map relevant objects to scene graph
    scene->obj_world_root->sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_world_root");
    scene->obj_camera_main->sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_camera_main");
    scene->obj_light_main->sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_light_main");
    scene->obj_skybox->sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_skybox");

    scene->obj_room->sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_room");
    scene->obj_room_box[0].sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_room_box_0");
    scene->obj_room_box[1].sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_room_box_1");
    scene->obj_room_sphere[0].sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_room_sphere_0");
    scene->obj_room_sphere[1].sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_room_sphere_1");
    scene->obj_room_lightbulb->sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_room_lightbulb");
    scene->obj_room_enclosure->sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_room_enclosure");

    scene->obj_material_container->sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_material_container");
    scene->obj_material_ball[0].sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_material_ball_0");
    scene->obj_material_ball[1].sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_material_ball_1");
    scene->obj_material_ball[2].sceneGraphIndex = a3hierarchyGetNodeIndex(scene->sceneGraph, "scene_material_ball_2");

    // scene graph state
    scene->sceneGraphState->hierarchy = 0;
    a3hierarchyStateCreate(scene->sceneGraphState, scene->sceneGraph);

    // room objects
    scene->obj_room_enclosure->position.z = (a3real)(8.0);

    scene->obj_room_lightbulb->position.z = (a3real)(16.0);

    scene->obj_room_box[0].position.x = (a3real)(-3.0);
    scene->obj_room_box[0].position.y = (a3real)(+2.0);
    scene->obj_room_box[0].position.z = (a3real)(2.5);
    scene->obj_room_box[0].euler.z = (a3real)(+45.0);

    scene->obj_room_box[1].position.x = (a3real)(-3.0);
    scene->obj_room_box[1].position.y = (a3real)(+2.0);
    scene->obj_room_box[1].position.z = (a3real)(7.5);
    scene->obj_room_box[1].euler.z = (a3real)(-30.0);

    scene->obj_room_sphere[0].position.x = (a3real)(+5.0);
    scene->obj_room_sphere[0].position.y = (a3real)(+2.0);
    scene->obj_room_sphere[0].position.z = (a3real)(2.0);

    scene->obj_room_sphere[1].position.x = (a3real)(+4.0);
    scene->obj_room_sphere[1].position.y = (a3real)(-4.0);
    scene->obj_room_sphere[1].position.z = (a3real)(1.0);

    scene->obj_material_container->position.x = (a3real)(-10.0);
    scene->obj_material_container->position.y = (a3real)(-15.0);
    scene->obj_material_ball[0].position.x = (a3real)( 0.0);
    scene->obj_material_ball[1].position.x = (a3real)(10.0);
    scene->obj_material_ball[2].position.x = (a3real)(20.0);

    // updates
    {
        void a3rendering_update_sceneGraph(a3_Scene_Rendering * scene, a3f64 const dt);
     
        a3rendering_update_sceneGraph(scene, 0.0);
    }

    //// other
    //a3hullReset(&scene->test_hull);
    //a3rayReset(&scene->test_ray);
    //scene->test_ray_param = a3real_zero;
    //scene->test_ray_fired = a3false;
    //scene->test_ray_hit   = a3false;
}


//-----------------------------------------------------------------------------

void a3rendering_input(a3_DemoState* demoState, a3_Scene_Rendering* scene, a3f64 const dt);
void a3rendering_update(a3_DemoState* demoState, a3_Scene_Rendering* scene, a3f64 const dt);
void a3rendering_render(a3_DemoState const* demoState, a3_Scene_Rendering const* scene, a3f64 const dt);
void a3rendering_input_keyCharPress(a3_DemoState const* demoState, a3_Scene_Rendering* scene, a3i32 const asciiKey, a3i32 const state);
void a3rendering_input_keyCharHold(a3_DemoState const* demoState, a3_Scene_Rendering* scene, a3i32 const asciiKey, a3i32 const state);

void a3rendering_loadValidate(a3_DemoState* demoState, a3_Scene_Rendering* scene)
{
	// initialize callbacks
	a3_SceneCallbacks* const callbacks = demoState->sceneCallbacks + demoState_modeRendering;
	callbacks->scene = scene;
	callbacks->handleInput =	(a3_Scene_EventCallback)		a3rendering_input;
	callbacks->handleUpdate =	(a3_Scene_EventCallback)		a3rendering_update;
	callbacks->handleRender =	(a3_Scene_EventCallbackConst)	a3rendering_render;
	callbacks->handleKeyPress = (a3_Scene_InputCallback)		a3rendering_input_keyCharPress;
	callbacks->handleKeyHold =	(a3_Scene_InputCallback)		a3rendering_input_keyCharHold;

	// initialize cameras dependent on viewport
	scene->proj_camera_main->aspect = demoState->frameAspect;
	a3scene_updateProjectorProjectionMat(scene->proj_camera_main, true);
	a3scene_setProjectorSceneObject(scene->proj_camera_main, scene->obj_camera_main);
	// initialize cameras not dependent on viewport
	
	// animation
	scene->sceneGraphState->hierarchy = scene->sceneGraph;
}


void a3rendering_load(a3_DemoState const* demoState, a3_Scene_Rendering* scene)
{
	a3ui32 i;

	a3_SceneObject* currentSceneObject;
	a3_SceneProjector* projector;


	// camera's starting orientation depends on "vertical" axis
	// we want the exact same view in either case
	const a3real sceneCameraAxisPos = 20.0f;
	const a3vec3 sceneCameraStartPos = {
		+sceneCameraAxisPos, //+ 10.0f,
		-sceneCameraAxisPos, //- 20.0f,
		+sceneCameraAxisPos +  5.0f,
	};
	const a3vec3 sceneCameraStartEuler = {
		-35.0f,
		  0.0f,
		 45.0f,
	};
	const a3f32 sceneObjectDistance = 8.0f;
	const a3f32 sceneObjectHeight = 2.0f;


	// all objects
	for (i = 0; i < renderingMaxCount_sceneObject; ++i)
		a3scene_initSceneObject(scene->object_scene + i);
	for (i = 0; i < renderingMaxCount_projector; ++i)
		a3scene_initProjector(scene->projector + i);

    // set up scales
	currentSceneObject = scene->obj_skybox;
	currentSceneObject->scaleMode = 1;
	currentSceneObject->scale.x = 256.0f;

	currentSceneObject = scene->obj_room_enclosure;
	currentSceneObject->scaleMode = 1;
    currentSceneObject->scale.x = 16.0f;

    currentSceneObject = scene->obj_room_lightbulb;
    currentSceneObject->scaleMode = 1;
    currentSceneObject->scale.x = 2.0f;

    currentSceneObject = &scene->obj_room_box[0];
    currentSceneObject->scaleMode = 1;
    currentSceneObject->scale.x = 5.0f;

    currentSceneObject = &scene->obj_room_box[1];
    currentSceneObject->scaleMode = 1;
    currentSceneObject->scale.x = 5.0f;

    currentSceneObject = &scene->obj_room_sphere[0];
    currentSceneObject->scaleMode = 1;
    currentSceneObject->scale.x = 2.0f;

    currentSceneObject = &scene->obj_room_sphere[1];
    currentSceneObject->scaleMode = 1;
    currentSceneObject->scale.x = 1.0f;

    currentSceneObject = &scene->obj_material_ball[0];
    currentSceneObject->scaleMode = 1;
    currentSceneObject->scale.x = 4.0f;

    currentSceneObject = &scene->obj_material_ball[1];
    currentSceneObject->scaleMode = 1;
    currentSceneObject->scale.x = 4.0f;

    currentSceneObject = &scene->obj_material_ball[2];
    currentSceneObject->scaleMode = 1;
    currentSceneObject->scale.x = 4.0f;

	// set up cameras
	projector = scene->proj_camera_main;
	projector->perspective = a3true;
	projector->fovy = a3real_fortyfive;
	projector->znear = 1.0f;
	projector->zfar = 1024.0f;
	projector->ctrlMoveSpeed = 10.0f;
	projector->ctrlRotateSpeed = 5.0f;
	projector->ctrlZoomSpeed = 5.0f;
	projector->sceneObject->position = sceneCameraStartPos;
	projector->sceneObject->euler = sceneCameraStartEuler;


	// set flags
	scene->render = rendering_renderPhongStereo;
	scene->display = rendering_displayStereo;
	scene->activeCamera = rendering_cameraSceneViewer;

	scene->pipeline = rendering_forward;
	scene->pass = rendering_passComposite;

	scene->targetIndex[rendering_passScene] = rendering_scene_finalcolor;
	scene->targetIndex[rendering_passComposite] = rendering_scene_finalcolor;

	scene->targetCount[rendering_passScene] = rendering_target_scene_max;
	scene->targetCount[rendering_passComposite] = 1;


	// setup
	a3rendering_init_scene(demoState, scene);
}


//-----------------------------------------------------------------------------
