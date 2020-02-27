models/weapons/shells/rifle-shell
{
	cull disable
	{
		map models/weapons/shells/rifle-shell.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/weapons/shells/rifle-shell.jpg
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
