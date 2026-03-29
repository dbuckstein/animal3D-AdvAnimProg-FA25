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
	
	drawStereoDisplay_fs4x.glsl
	Output final stereoscopic display.
*/

#version 450

in vbVertexData {
	vec4 vTexcoord_atlas;
};

uniform sampler2D uImage00;

layout (location = 0) out vec4 rtFragColor;

void main()
{
	// DUMMY OUTPUT: texture sample
	vec4 col = texture(uImage00, vTexcoord_atlas.xy);
	rtFragColor = col;
}
