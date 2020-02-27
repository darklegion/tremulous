models/buildables/reactor/reactor_main
{
	{
		map models/buildables/reactor/reactor_main.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/reactor/reactor_glow.tga
		blendfunc add
		rgbGen wave sin 0 1 0 0.5
	}
	{
		map models/buildables/reactor/reactor_main.tga
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
models/buildables/reactor/reactor_body
{
	{
		map models/buildables/reactor/reactor_body.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/reactor/reactor_body.tga
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
models/buildables/reactor/reactor_parts
{
	{
		map models/buildables/reactor/reactor_parts.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/reactor/reactor_parts.tga
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
models/buildables/reactor/reactor_top
{
	{
		map models/buildables/reactor/reactor_top.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/reactor/reactor_top.tga
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
models/buildables/reactor/reactor_control
{
	{
		map models/buildables/reactor/reactor_control.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/reactor/reactor_control.tga
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

models/buildables/reactor/reactor_meter
{
	{
		map models/buildables/reactor/reactor_meter.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/reactor/reactor_meter.tga
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

models/buildables/reactor/reactor_display
{
	{
		map models/buildables/reactor/reactor_display.tga
	}
	{
		map models/buildables/mgturret/ref_map.jpg
		blendFunc GL_DST_COLOR GL_ONE
		detail
		tcGen environment
	}
}

models/buildables/reactor/reactor_bolt
{
	cull disable
	{
		map models/buildables/reactor/reactor_bolt.tga
		blendfunc add
		rgbGen identity
		tcMod scroll 2 0
	}
}
