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


uniform vec4 uAxis;


vec2 barrel(in vec2 c, in vec2 c0)
{
	vec2 dc  = c - c0;
	float k1 = 1.0e-6;
	float r2 = dot(dc, dc);
	return c0 + dc * (1.0 + r2 * k1);
}


vec4 barrelStereo(in vec2 texcoord)
{
	vec4 col = vec4(0.0, 0.0, 0.0, 1.0);
	if (texcoord.x < 0.5)
	{
		const vec2 c0_l = vec2(0.25, 0.50);
		texcoord = barrel(texcoord * uAxis.xy, c0_l * uAxis.xy) * uAxis.zw;
		if ((texcoord.x >= 0.0) && (texcoord.x < 0.5) &&
			(texcoord.y >= 0.0) && (texcoord.y < 1.0))
			col = texture(uImage00, texcoord);
	}
	else
	{
		const vec2 c0_r = vec2(0.75, 0.50);
		texcoord = barrel(texcoord * uAxis.xy, c0_r * uAxis.xy) * uAxis.zw;
		if ((texcoord.x >= 0.5) && (texcoord.x < 1.0) &&
			(texcoord.y >= 0.0) && (texcoord.y < 1.0))
			col = texture(uImage00, texcoord);
	}
	return col;
}


void main()
{
	//// DUMMY OUTPUT: texture sample
	//vec4 col = texture(uImage00, vTexcoord_atlas.xy);
	//rtFragColor = col;

	rtFragColor = barrelStereo(vTexcoord_atlas.xy);
}
