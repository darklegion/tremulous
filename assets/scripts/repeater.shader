models/buildables/repeater/repeator_body
{
	{
		map models/buildables/repeater/repeator_body.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/repeater/repeator_body.jpg
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
models/buildables/repeater/energy
{
	{
		map models/buildables/repeater/energy.tga
		rgbGen wave sawtooth 0.3 1 0 0.5
		tcMod scale 2 1
		tcMod scroll 0 1
	}
}

models/buildables/repeater/repeator_panel
{
	{
		map models/buildables/repeater/repeator_panel.tga
		rgbGen identity
	}
}
