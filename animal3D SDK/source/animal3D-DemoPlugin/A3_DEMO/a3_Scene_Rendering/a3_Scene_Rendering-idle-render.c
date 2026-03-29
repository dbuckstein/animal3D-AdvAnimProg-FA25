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
		//"Ray-tracing",
        "Phong shading (stereoscopic)"
	};

	// forward display names
	a3byte const* displayProgramName[rendering_display_max] = {
		"Texture",
        "Stereo"
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

    //// tests
    //a3textDraw(text, textAlign, textOffset += textOffsetDelta, textDepth, col.r, col.g, col.b, col.a,
    //    "    Test ray fired: %s", (scene->test_ray_fired ? "TRUE " : "FALSE"));
    //a3textDraw(text, textAlign, textOffset += textOffsetDelta, textDepth, col.r, col.g, col.b, col.a,
    //    "        Hit: %s; t=%lf", (scene->test_ray_hit ? "TRUE " : "FALSE"), (a3f64)scene->test_ray_param);
}


//-----------------------------------------------------------------------------

void a3demo_uploadTransformStacks(
    a3_UniformBuffer const* ubo_transform_stacks,
    a3_SceneModelMatrixStack const* model_matrix_stacks, a3_SceneViewerMatrixStack const* viewer_matrix_stacks,
    a3ui32 const max_models, a3ui32 const num_models, a3ui32 const max_viewers, a3ui32 const num_viewers);

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

        demoState->draw_node,           // materials root
        demoState->draw_unit_sphere,    // material balls
        demoState->draw_unit_sphere,    // 
        demoState->draw_unit_sphere,    // 
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

        0,                              // materials root
        demoState->tex_checker,			// materia balls
        demoState->tex_checker,			// 
        demoState->tex_checker,			// 
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

        0,
        0,
        0,
        0,
    };

    // program override
    const a3_SceneShaderProgram* render_program_override[] = {
        0,
        0,
        0,
        0,

        0,
        0,
        0,
        0,
        0,
        0,
        0,

        0,
        demoState->prog_drawPhotorealistic0,
        demoState->prog_drawPhotorealistic1,
        demoState->prog_drawPhotorealistic2,
    };

    // texture sets
    const a3_Texture* texture_set[][8] = {
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        
        { 0 },
        { demoState->tex_earth_dm, demoState->tex_earth_sm, demoState->tex_earth_nm, demoState->tex_earth_hm, demoState->tex_earth_cloud, demoState->tex_earth_light, demoState->tex_checker, 0 },
        { demoState->tex_earth_dm, demoState->tex_earth_sm, demoState->tex_earth_nm, demoState->tex_earth_hm, demoState->tex_earth_cloud, demoState->tex_earth_light, demoState->tex_checker, 0 },
        { demoState->tex_earth_dm, demoState->tex_earth_sm, demoState->tex_earth_nm, demoState->tex_earth_hm, demoState->tex_earth_cloud, demoState->tex_earth_light, demoState->tex_checker, 0 },
    };
    a3ui32 const max_texture_set_size = sizeof(*texture_set) / sizeof(**texture_set);

	// forward pipeline shader programs
	const a3_SceneShaderProgram* renderProgram[rendering_pipeline_max][rendering_render_max] = {
		{
			demoState->prog_drawColorUnif,
			demoState->prog_drawTexture,
			demoState->prog_drawLambert,
			demoState->prog_drawPhong,
            //demoState->prog_drawRT,
            demoState->prog_drawPhongStereo,
        },
	};

	// display shader programs
	const a3_SceneShaderProgram* displayProgram[rendering_display_max] = {
		demoState->prog_drawTexture,
        demoState->prog_drawStereoDisplay,
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
		case rendering_renderPhongStereo:
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
        /*case rendering_renderRT:
        {
            a3demo_uploadTransformStacks(demoState->ubo_transformStack,
                &scene->modelMatrixStack[scene->obj_room_box - scene->object_scene],
                &scene->viewerMatrixStack[scene->proj_camera_main - scene->projector],
                renderingMaxCount_sceneObject, (a3ui32)(scene->obj_room_enclosure - scene->obj_room),
                renderingMaxCount_projector, 1);
            a3shaderUniformBufferActivate(demoState->ubo_transformStack, 0);
            for (currentSceneObject = scene->obj_room_enclosure, endSceneObject = scene->obj_room_enclosure,
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

            // Materials
            {
                a3_SceneViewerMatrixStack const* const camera_viewer = &scene->viewerMatrixStack[rendering_cameraSceneViewer];
                a3_SceneModelMatrixStack  const* const camera_model  = &scene->modelMatrixStack[activeCameraObject - scene->object_scene];

                a3demo_uploadTransformStacks(demoState->ubo_transformStack,
                    &scene->modelMatrixStack[scene->obj_material_ball - scene->object_scene],
                    &scene->viewerMatrixStack[scene->proj_camera_main - scene->projector],
                    renderingMaxCount_sceneObject, (a3ui32)(&scene->obj_material_ball[3] - scene->obj_material_container),
                    renderingMaxCount_projector, 1);
                a3shaderUniformBufferActivate(demoState->ubo_transformStack, 0);
                for (currentSceneObject = &scene->obj_material_ball[0], endSceneObject = &scene->obj_material_ball[2],
                    j = (a3ui32)(currentSceneObject - scene->object_scene);
                    currentSceneObject <= endSceneObject;
                    ++j, ++currentSceneObject)
                {
                    a3_SceneModelMatrixStack const* const model = &scene->modelMatrixStack[j];

                    // override program
                    currentDemoProgram = render_program_override[j];
                    a3shaderProgramActivate(currentDemoProgram->program);
                    
                    // ****TO-DO-RTR-PROJECT-3: 
                    // SET UP AND UPLOAD ADDITIONAL PERTINENT UNIFORMS
                    //  -> this may require setting up more textures in the demo state
                    //  -> use existing uniform handles and uniform buffers if possible
                    //     to avoid having to set up more of them

                    // activate textures
                    for (a3ui32 tid = 0; tid < max_texture_set_size; ++tid)
                    {
                        a3textureActivate(texture_set[j][tid], a3tex_unit00 + tid);
                    }

                    a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uP, 1, camera_viewer->projectionMat.mm);
                    a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uP_inv, 1, camera_viewer->projectionMatInverse.mm);
                    a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uPB, 1, camera_viewer->projectionBiasMat.mm);
                    a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uPB_inv, 1, camera_viewer->projectionBiasMatInverse.mm);
                    a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uAtlas, 1, a3mat4_identity.mm);
                    a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, hueCount, rgba4->v);
                    if (demoState->updateAnimation)
                        a3shaderUniformSendDouble(a3unif_single, currentDemoProgram->uTime, 1, &demoState->timer_display->totalTime);
                    a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uAxis, 1, pixelSizeAndInv.v);

                    // send data and draw
                    i = (j * 2 + 11) % hueCount;
                    currentDrawable = drawable[currentSceneObject - scene->obj_world_root];
                    a3real4x4Product(modelViewMat.m, camera_model->modelMatInverse.m, model->modelMat.m);
                    a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMV, 1, modelViewMat.mm);
                    a3scene_quickInvertTranspose_internal(modelViewMat.m);//manual inverse here
                    modelViewMat.v3 = a3vec4_zero;
                    a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMV_nrm, 1, modelViewMat.mm);
                    a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, rgba4[i].v);
                    a3shaderUniformSendInt(a3unif_single, currentDemoProgram->uIndex, 1, &j);
                    a3vertexDrawableActivateAndRender(currentDrawable);
                }
            }
            break;
        }*/
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
	if ((scene->pass >= rendering_passScene) && (scene->render != rendering_renderPhongStereo))
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
            //// ray fired
            //if (scene->test_ray_fired)
            //{
            //    a3_SceneProjector const* projector = scene->projector + scene->activeCamera;
            //    a3real const* const color = scene->test_ray_hit ? green : red;
            //    a3real const param = scene->test_ray_hit ? scene->test_ray_param : a3real_four;
            //
            //    modelMat = projector->sceneObject->modelMat;
            //    a3real3MulS(modelMat.v0.v, (a3real)(0.025));
            //    a3real3MulS(modelMat.v1.v, (a3real)(0.025));
            //    a3real3MulS(modelMat.v2.v, (a3real)(0.025));
            //    a3rayComputePos(modelMat.v3.v, scene->test_ray.p_origin.v, scene->test_ray.v_direction.v, a3real_zero);
            //    a3scene_drawModelSolidColor(modelViewProjectionMat.m, viewProjectionMat.m, modelMat.m, demoState->prog_drawColorUnif, demoState->draw_unit_sphere, blue);
            //    a3rayComputePos(modelMat.v3.v, scene->test_ray.p_origin.v, scene->test_ray.v_direction.v, param);
            //    a3scene_drawModelSolidColor(modelViewProjectionMat.m, viewProjectionMat.m, modelMat.m, demoState->prog_drawColorUnif, demoState->draw_unit_sphere, color);
            //}
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
