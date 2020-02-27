models/buildables/dcc/comp_pipes
{
	{
		map models/buildables/dcc/comp_pipes.tga
		tcGen environment
		rgbGen lightingDiffuse
	}
	{
		map $whiteimage
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

models/buildables/dcc/comp_front
{
	{
		map models/buildables/dcc/comp_front.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/dcc/comp_front.jpg
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

models/buildables/dcc/comp_key
{
	{
		map models/buildables/dcc/comp_key.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/dcc/comp_key.jpg
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

models/buildables/dcc/comp_parts
{
	{
		map models/buildables/dcc/comp_parts.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/dcc/comp_parts.jpg
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

models/buildables/dcc/comp_top
{
	{
		map models/buildables/dcc/comp_top.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/dcc/comp_top.jpg
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

models/buildables/dcc/comp_display
{
	{
		map models/buildables/dcc/comp_grad.tga
		tcMod scroll 0 1
	}
	{
		map models/buildables/dcc/comp_display.tga
		blendfunc gl_one gl_src_alpha
	}
	{
		map models/buildables/mgturret/ref_map.jpg
		blendFunc GL_DST_COLOR GL_ONE
		detail
		tcGen environment
	}
}
