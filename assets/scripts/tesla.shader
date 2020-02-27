models/buildables/tesla/tesla_main
{
	{
		map models/buildables/tesla/tesla_main.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/tesla/tesla_main.tga
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

models/buildables/tesla/tesla_ball
{
	{
		map models/buildables/tesla/tesla_ball.tga
		rgbGen lightingDiffuse
		tcMod scroll -0.2 0.2
	}
	{
		map models/buildables/tesla/tesla_ball.tga
		rgbGen lightingDiffuse
		blendfunc add
		detail
		tcMod scroll 0.35 -0.45
		tcMod scale -1 1
	}
	{
		map models/buildables/tesla/tesla_ball.tga
		tcGen environment
		blendFunc GL_SRC_ALPHA GL_ONE
		alphaGen lightingSpecular
	}
}

models/buildables/tesla/tesla_grill
{
	{
		map models/buildables/tesla/tesla_grill.tga
		rgbGen wave sin 0 1 0 0.4
	}
}

models/buildables/tesla/tesla_spark
{
	cull disable
	{
		map models/buildables/tesla/tesla_spark.tga
		blendfunc add
		rgbGen identity
	}
}


models/ammo/tesla/tesla_bolt
{
	cull disable
	{
		map models/ammo/tesla/tesla_bolt.tga
		blendfunc add
		rgbGen vertex
		tcMod scroll 0.2 0
	}
	{
		map models/ammo/tesla/tesla_bolt.tga
		blendfunc add
		rgbGen wave sin 0 1 0 5
		tcMod scroll 0.5 0
		tcMod scale -1 1
	}
}
