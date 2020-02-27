models/weapons/grenade/grenade
{
	cull disable
	{
		map models/weapons/grenade/grenade.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/weapons/grenade/grenade.jpg
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

models/weapons/grenade/grenade_s
{
	{
		map models/weapons/grenade/energy.jpg
		rgbGen wave sawtooth 0.3 1 0 0.5
		tcMod scale 2 1
		tcMod scroll 0 1
	}
}

gfx/grenade/flare_01
{
	{
		map gfx/grenade/flare_01.tga
		blendfunc add
	}
}
