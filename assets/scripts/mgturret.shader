models/buildables/mgturret/t_flash
{
	cull disable
	{
		map models/buildables/mgturret/t_flash.tga
		blendfunc add
		rgbGen wave square 0 1 0 10
	}
}

models/buildables/mgturret/turret_coil
{
	cull disable
	{
		map models/buildables/mgturret/turret_coil.tga
		rgbGen lightingDiffuse
		alphaFunc GE128
		depthWrite
	}
	{
		map models/buildables/mgturret/turret_coil.tga
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

models/buildables/mgturret/turret_shiny
{
	{
		map models/buildables/mgturret/turret_shiny.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/mgturret/turret_shiny.tga
		blendFunc GL_SRC_ALPHA GL_ONE
		detail
		alphaGen lightingSpecular
	}
	{
		map models/buildables/mgturret/ref_map.jpg
		blendFunc GL_DST_COLOR GL_ONE
		detail
		tcGen environment
	}
}
models/buildables/mgturret/turret_barrel
{
	{
		map models/buildables/mgturret/turret_barrel.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/mgturret/turret_barrel.jpg
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

models/buildables/mgturret/turret_base
{
	{
		map models/buildables/mgturret/turret_base.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/mgturret/turret_base.jpg
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

models/buildables/mgturret/turret_top
{
	{
		map models/buildables/mgturret/turret_top.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/mgturret/turret_top.jpg
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
