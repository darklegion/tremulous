models/buildables/arm/arm_body
{
	{
		map models/buildables/arm/arm_body.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/arm/arm_body.jpg
		blendFunc GL_SRC_ALPHA GL_ONE
		detail
		alphaGen lightingSpecular
	}
	{
		map models/buildables/mgturret/ref_map.jpg
		blendFunc GL_DST_COLOR GL_ONE
		detail
		tcGen environment
		alphaGen lightingSpecular
	}
}

models/buildables/arm/arm_parts
{
	{
		map models/buildables/arm/arm_parts.jpg
		rgbGen lightingDiffuse
	}
	{
		map $whiteimage
		blendFunc GL_SRC_ALPHA GL_ONE
		detail
		alphaGen lightingSpecular
	}
	{
		map models/buildables/mgturret/ref_map.jpg
		blendFunc GL_DST_COLOR GL_ONE
		detail
		tcGen environment
		alphaGen lightingSpecular
	}
}
