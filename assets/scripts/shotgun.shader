models/weapons/shotgun/shotgun
{
	cull disable
	{
		map models/weapons/shotgun/shotgun.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/weapons/shotgun/shotgun.jpg
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
