models/weapons/flamer/pilot
{
	sort additive
	cull disable
	{
		map models/weapons/flamer/pilot.jpg
		blendfunc GL_ONE GL_ONE
		tcMod scroll 9.0 0
	}
}

models/weapons/flamer/gas
{
	sort additive
	cull disable
	{
		map models/weapons/flamer/gas.jpg
		blendfunc GL_ONE GL_ONE
		tcMod scroll 0.04 0.02
	}
}

models/weapons/flamer/flamer
{
	sort additive
//	cull disable
	surfaceparm trans
	{
		map models/weapons/flamer/flamer.tga
		depthWrite
		alphaFunc GE128
		rgbGen lightingDiffuse
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	{
		map models/weapons/flamer/flamer.tga
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
