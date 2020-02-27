models/weapons/chaingun/chaingun
{
	cull disable
	{
		map models/weapons/chaingun/chaingun.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/weapons/chaingun/chaingun.jpg
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

models/weapons/chaingun/barrels
{
	cull disable
	{
		map models/weapons/chaingun/barrels.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/weapons/chaingun/barrels.jpg
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
