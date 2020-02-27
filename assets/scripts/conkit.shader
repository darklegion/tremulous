models/weapons/ckit/ckit
{
	cull disable
	{
		map models/weapons/ckit/ckit.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/weapons/ckit/ckit.jpg
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

models/weapons/ckit/rep_cyl
{
	cull disable
	{
		map models/weapons/ckit/rep_cyl.jpg
		blendfunc add
		rgbGen lightingDiffuse
		tcMod scroll 0.2 0
	}
	{
		map models/weapons/ckit/lines2.jpg
		blendfunc add
		rgbGen identity
		tcMod scroll 0 -0.2
	}
}

models/weapons/ckit/screen
{
	noPicMip
	{
		map models/weapons/ckit/screen.jpg
	}
	{
		map models/weapons/ckit/scroll.jpg
		blendfunc add
		tcMod scroll 10 -0.4
	}
	{
		map models/buildables/mgturret/ref_map.jpg
		blendFunc GL_DST_COLOR GL_ONE
		detail
		tcGen environment
	}
}

models/weapons/ackit/rep_cyl
{
	cull disable
	{
		map models/weapons/ackit/rep_cyl.jpg
		blendfunc add
		rgbGen lightingDiffuse
		tcMod scroll 0.2 0
	}
	{
		map models/weapons/ackit/lines2.jpg
		blendfunc add
		rgbGen identity
		tcMod scroll 0 -0.2
	}
}

models/weapons/ackit/advscreen
{
	noPicMip
	{
		map models/weapons/ackit/advscreen.jpg
	}
	{
		map models/weapons/ackit/scroll.jpg
		blendfunc add
		tcMod scroll 10 -0.4
	}
	{
		map models/buildables/mgturret/ref_map.jpg
		blendFunc GL_DST_COLOR GL_ONE
		detail
		tcGen environment
	}
}
