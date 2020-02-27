models/buildables/trapper/trapper_gills
{
cull disable
	{
		map models/buildables/trapper/trapper_gills.tga
		rgbGen lightingDiffuse
		alphaFunc GE128
	}
	{
		map models/buildables/trapper/trapper_gills.tga
		rgbGen lightingDiffuse
		tcMod scroll -0.01 0
		tcMod scale -1 1
		alphaFunc GE128
	}
}

models/buildables/trapper/trapper_body
{
	{
		map models/buildables/trapper/trapper_body.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/trapper/trapper_body.jpg
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


models/buildables/trapper/trapper_tent
{
	{
		map models/buildables/trapper/trapper_tent.jpg
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/trapper/trapper_tent.jpg
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
