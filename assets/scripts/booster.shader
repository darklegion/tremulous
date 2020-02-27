models/buildables/booster/booster_head
{
	{
		map models/buildables/booster/booster_head.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/booster/booster_head.tga
		blendFunc GL_SRC_ALPHA GL_ONE
		detail
		alphaGen lightingSpecular
	}
	{
		map models/buildables/booster/ref_map.tga
		blendfunc filter
		rgbGen identity
		tcMod rotate 5
		tcGen environment
	}
}

models/buildables/booster/booster_sac
{
	{
		map models/buildables/booster/booster_sac.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/booster/poison.tga
		blendfunc add
		rgbGen wave sin 0 1 0 0.1
		tcMod scroll -0.05 -0.05
	}
	{
		map models/buildables/booster/booster_sac.tga
		blendFunc GL_SRC_ALPHA GL_ONE
		detail
		alphaGen lightingSpecular
	}
}
models/buildables/booster/pod_strands
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
