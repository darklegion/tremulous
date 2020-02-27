models/buildables/telenode/telenode_top
{
	{
		map models/buildables/telenode/telenode_top.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/telenode/telenode_top.tga
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

models/buildables/telenode/energy
{
	{
		map models/buildables/telenode/energy.tga
		rgbGen wave inversesawtooth 0.2 0.4 0 1
		tcMod rotate 10
	}
}

models/buildables/telenode/rep_cyl
{
	cull disable
	{
		map models/buildables/telenode/rep_cyl.jpg
		blendfunc add
		rgbGen lightingDiffuse
		tcMod scroll 0.2 0
	}
	{
		map models/buildables/telenode/lines2.tga
		blendfunc add
		rgbGen identity
		tcMod scroll 0 0.2
	}
}

models/buildables/telenode/telenode_parts
{
	{
		map models/buildables/telenode/telenode_parts.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/telenode/telenode_parts.tga
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
