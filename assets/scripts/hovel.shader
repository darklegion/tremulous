models/buildables/hovel/hovel
{
	{
		map models/buildables/hovel/hovel.tga
		rgbGen lightingDiffuse
	}
  {
		map models/buildables/hovel/hovel.tga
		blendFunc GL_SRC_ALPHA GL_ONE
		detail
		alphaGen lightingSpecular
	}
}

models/buildables/hovel/hovel_front
{
	{
		map models/buildables/hovel/hovel_front.tga
		rgbGen lightingDiffuse
	}
  {
		map models/buildables/hovel/hovel_front.tga
		blendFunc GL_SRC_ALPHA GL_ONE
		detail
		alphaGen lightingSpecular
	}
}

models/buildables/hovel/pod_strands
{
	cull disable
	{
		map models/buildables/barricade/pod_strands.tga
		rgbGen lightingDiffuse
		alphaFunc GE128
		depthWrite
	}
	{
		map models/buildables/barricade/pod_strands.tga
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
