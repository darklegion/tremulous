models/players/human_base/h_base
{
  cull disable
  {
    map models/players/human_base/h_base.tga
    rgbGen lightingDiffuse
    alphaFunc GE128
    depthWrite
  }
  {
    map models/players/human_base/h_base.tga
    blendFunc GL_SRC_ALPHA GL_ONE
    detail
    alphaGen lightingSpecular
    depthFunc equal
  }
}

models/players/human_base/h_helmet
{
  cull disable
  {
    map models/players/human_base/h_helmet.tga
    rgbGen lightingDiffuse
    alphaFunc GE128
    depthWrite
  }
  {
    map models/players/human_base/h_helmet.tga
    blendFunc GL_SRC_ALPHA GL_ONE
    detail
    alphaGen lightingSpecular
    depthFunc equal
  }
  {  // TODO: Should separate metalics parts
    map models/buildables/mgturret/ref_map.jpg
    blendFunc GL_DST_COLOR GL_ONE
    detail
    tcGen environment
    alphaGen lightingSpecular
    depthFunc equal
  }
}
models/players/human_base/battpack
{
  cull disable
  {
    map models/players/human_base/battpack.jpg
    rgbGen lightingDiffuse
    alphaFunc GE128
    depthWrite
  }
  {
    models/players/human_base/battpack.jpg
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

models/players/human_base/base
{
  cull disable
  {
    map models/players/human_base/base.tga
    rgbGen lightingDiffuse
    alphaFunc GE128
    depthWrite
  }
  {
    map models/players/human_base/base.tga
    blendFunc GL_SRC_ALPHA GL_ONE
    detail
    alphaGen lightingSpecular
    depthFunc equal
  }
  // {  // TODO: Should separate metalics parts
  //   map models/buildables/mgturret/ref_map.jpg
  //   blendFunc GL_DST_COLOR GL_ONE
  //   detail
  //   tcGen environment
  //   alphaGen lightingSpecular
  //   depthFunc equal
  // }
}

models/players/human_base/light
{
  cull disable
  {
    map models/players/human_base/light.tga
    rgbGen lightingDiffuse
    alphaFunc GE128
    depthWrite
  }
  {
    map models/players/human_base/light.tga
    blendFunc GL_SRC_ALPHA GL_ONE
    detail
    alphaGen lightingSpecular
    depthFunc equal
  }
  // {  // TODO: Should separate metalics parts
  //   map models/buildables/mgturret/ref_map.jpg
  //   blendFunc GL_DST_COLOR GL_ONE
  //   detail
  //   tcGen environment
  //   alphaGen lightingSpecular
  //   depthFunc equal
  // }
}

models/players/human_base/armour
{
  cull disable
  {
    map models/players/human_base/armour.jpg
    rgbGen lightingDiffuse
    alphaFunc GE128
    depthWrite
  }
  {
    map models/players/human_base/armour.jpg
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

models/players/human_base/shoulderpads
{
  cull disable
  {
    map models/players/human_base/shoulderpads.jpg
    rgbGen lightingDiffuse
    alphaFunc GE128
    depthWrite
  }
  {
    map models/players/human_base/shoulderpads.jpg
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

models/players/human_bsuit/human_bsuit
{
  cull disable
  {
    map models/players/human_bsuit/human_bsuit.tga
    rgbGen lightingDiffuse
    alphaFunc GE128
    depthWrite
  }
  {
    map models/players/human_bsuit/human_bsuit.tga
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
