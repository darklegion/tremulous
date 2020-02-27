models/players/human_base/jetpack
{
	cull disable
	{
		map models/players/human_base/jetpack.tga
		rgbGen lightingDiffuse
		alphaFunc GE128
		depthWrite
	}
	{
		map models/players/human_base/jetpack.tga
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

models/players/human_base/jetpack_flash
{
	sort additive
	{
		map	models/players/human_base/jetpack_flash.jpg
		blendfunc GL_ONE GL_ONE
		tcMod scroll 10.0 0.0
	}
}
