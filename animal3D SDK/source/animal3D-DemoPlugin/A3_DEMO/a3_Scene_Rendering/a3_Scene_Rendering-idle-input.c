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

	a3_Scene_Rendering-idle-input.c/.cpp
	Demo mode implementations: rendering scene.

	********************************************
	*** INPUT FOR RENDERING SCENE MODE       ***
	********************************************
*/

//-----------------------------------------------------------------------------

#include "../a3_Scene_Rendering.h"

//typedef struct a3_DemoState a3_DemoState;
#include "../a3_DemoState.h"

#include "../_a3_scene_utilities/a3_SceneMacros.h"


//-----------------------------------------------------------------------------
// CALLBACKS

// main demo mode callback
void a3rendering_input_keyCharPress(a3_DemoState const* demoState, a3_Scene_Rendering* scene, a3i32 const asciiKey, a3i32 const state)
{
	switch (asciiKey)
	{
		// toggle render program
		a3sceneCtrlCasesLoop(scene->render, rendering_render_max, 'k', 'j');

		// toggle display program
		a3sceneCtrlCasesLoop(scene->display, rendering_display_max, 'K', 'J');

		// toggle active camera
		a3sceneCtrlCasesLoop(scene->activeCamera, rendering_camera_max, 'v', 'c');

		// toggle pipeline mode
		a3sceneCtrlCasesLoop(scene->pipeline, rendering_pipeline_max, ']', '[');

		// toggle target
		a3sceneCtrlCasesLoop(scene->targetIndex[scene->pass], scene->targetCount[scene->pass], '}', '{');

		// toggle pass to display
		a3sceneCtrlCasesLoop(scene->pass, rendering_pass_max, ')', '(');

		// toggle control target
		a3sceneCtrlCasesLoop(scene->ctrl_target, rendering_ctrlmode_max, '\'', ';');

		//// toggle position input mode
		//a3sceneCtrlCasesLoop(scene->ctrl_position, rendering_inputmode_max, '=', '-');
		
		//// toggle rotation input mode
		//a3sceneCtrlCasesLoop(scene->ctrl_rotation, rendering_inputmode_max, '+', '_');

    //case '/':
    //    a3rayReset(&scene->test_ray);
    //    scene->test_ray_param = a3real_zero;
    //    scene->test_ray_fired = a3false;
    //    scene->test_ray_hit   = a3false;
    //    break;
	}
}

void a3rendering_input_keyCharHold(a3_DemoState const* demoState, a3_Scene_Rendering* scene, a3i32 const asciiKey, a3i32 const state)
{
//	switch (asciiKey)
//	{
//
//	}
}


//-----------------------------------------------------------------------------

void a3demo_input_controlObject(
	a3_DemoState* demoState, a3_SceneObject* object,
	a3f64 const dt, a3real ctrlMoveSpeed, a3real ctrlRotateSpeed);
void a3demo_input_controlProjector(
	a3_DemoState* demoState, a3_SceneProjector* projector,
	a3f64 const dt, a3real ctrlMoveSpeed, a3real ctrlRotateSpeed, a3real ctrlZoomSpeed);

void a3rendering_input(a3_DemoState* demoState, a3_Scene_Rendering* scene, a3f64 const dt)
{
	a3_SceneProjector* projector = scene->projector + scene->activeCamera;
	//a3_SceneObject* sceneObject;

	// right click to ray pick
	if (a3mouseGetState(demoState->mouse, a3mouse_right) == a3input_down)
	{
		// get window coordinates
		a3i32 const x = a3mouseGetX(demoState->mouse) + demoState->frameBorder;
		a3i32 const y = a3mouseGetY(demoState->mouse) + demoState->frameBorder;

		// transform to NDC on near plane
		a3vec4 coord = a3vec4_one;
		coord.x = +((a3real)x * demoState->frameWidthInv * a3real_two - a3real_one);
		coord.y = -((a3real)y * demoState->frameHeightInv * a3real_two - a3real_one);
		coord.z = -a3real_one;

        // transform to world space
        a3real4Real4x4Mul(projector->viewProjectionMatInv.m, coord.v);

        // perspective multiply (divide by w component inverse)
        a3real4DivS(coord.v, coord.w);

        //// ****TO-DO-RTR-PROJECT-2: TEST RAY HIT
        //a3rayInitTargetUnit(&scene->test_ray, projector->sceneObject->modelMat.v3.v, coord.v);
        ////a3rayInitTarget(&scene->test_ray, projector->sceneObject->modelMat.v3.v, coord.v);
        //scene->test_ray_fired = a3true;
        //{
        //    // Test collision with one of the scene objects, e.g. sphere.
        //    // Knowns: origin and direction of ray; known point of reference; shape dimensions.
        //    a3vec3 const P0 = scene->test_ray.p_origin.xyz;      //ray origin
        //    a3vec3 const p  = scene->test_ray.v_direction.xyz;   //ray direction (assume normalized here)
        //    a3vec3 const Q0 = scene->obj_room_sphere[0].position;//object center
        //    a3real const r  = scene->obj_room_sphere[0].scale.x; //sphere radius (stored here as scale)
        //
        //    (void)P0;
        //    (void)p;
        //    (void)Q0;
        //    (void)r;
        //}
	}
	
	// choose control target
	switch (scene->ctrl_target)
	{
	case rendering_ctrl_camera:
		// move camera
		a3demo_input_controlProjector(demoState, projector,
			dt, projector->ctrlMoveSpeed, projector->ctrlRotateSpeed, projector->ctrlZoomSpeed);
		break;
	}

	// allow the controller, if connected, to change control targets
	if (a3XboxControlIsConnected(demoState->xcontrol))
	{
		if (a3XboxControlIsPressed(demoState->xcontrol, a3xbox_DPAD_right))
			a3sceneCtrlIncLoop(scene->ctrl_target, rendering_ctrlmode_max);
		if (a3XboxControlIsPressed(demoState->xcontrol, a3xbox_DPAD_left))
			a3sceneCtrlDecLoop(scene->ctrl_target, rendering_ctrlmode_max);
	}
}


//-----------------------------------------------------------------------------
