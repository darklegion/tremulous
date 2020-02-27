models/buildables/acid_tube/acid_tube
{
	{
		map models/buildables/acid_tube/acid_tube.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/acid_tube/acid_tube.jpg
		blendFunc GL_SRC_ALPHA GL_ONE
		detail
		alphaGen lightingSpecular
	}
}

models/buildables/acid_tube/acid_tube_inside
{
	{
		map models/buildables/acid_tube/acid_tube_inside.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/acid_tube/acid_tube_inside.jpg
		blendFunc GL_SRC_ALPHA GL_ONE
		detail
		alphaGen lightingSpecular
	}
}

models/buildables/acid_tube/pod_strands
{
	cull disable
	{
		map models/buildables/eggpod/pod_strands.tga
		rgbGen lightingDiffuse
		alphaFunc GE128
		depthWrite
	}
	{
		map models/buildables/eggpod/pod_strands.tga
		blendFunc GL_SRC_ALPHA GL_ONE
		detail
		alphaGen lightingSpecular
		depthFunc equal
	}
	{
		map models/buildables/mgturret/ref_map.jpg
		blendFunc GL_DST_COLOR GL_ONE
		detail
		tcGen environment
		alphaGen lightingSpecular
		depthFunc equal
	}
}
