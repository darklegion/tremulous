models/buildables/hive/hive
{
	{
		map models/buildables/hive/hive.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/hive/hive.tga
		blendFunc GL_SRC_ALPHA GL_ONE
		detail
		alphaGen lightingSpecular
	}
}

models/buildables/hive/hive_strands_s
{
	cull disable
	{
		map models/buildables/hive/hive_strands.tga
		rgbGen lightingDiffuse
		alphaFunc GE128
		depthWrite
	}
	{
		map models/buildables/hive/hive_strands.tga
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
