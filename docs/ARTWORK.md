## Artwork Look and Effects

Skyscraper allows you to fully customize how you want the final frontend artwork to appear by editing the file `/home/<USER>/.skyscraper/artwork.xml`. Each piece of artwork can be composited from one or more of the other artwork resources and you can even apply several effects to them.

-   Each `<output>` node exports a single piece of artwork.
-   If no `<layer>` nodes are nested in an output node, it will simply export the defined type as raw unmanipulated artwork.
-   All layer nodes and their nested effects (outmost to innermost) are rendered / applied top-to-bottom.

Read on for an example `artwork.xml` and a more thorough description of the `<output>`, `<layer>` and the various available effect nodes.

!!! warning

    Some users seem to have a habbit of editing the `/home/<USER>/skysource/artwork.xml` file. _This is not the correct file!_ Please read the first paragraph. :)

!!! tip

     Every time you change the artwork configuration, remember to regenerate the game list. Your changes won't take effect until you do so. Read more about this [here](USECASE.md#the-game-list-generation-phase).

Watch a video demonstrating the artwork compositing features [here](https://youtu.be/TIDD8EFSz50). Note that the video is quite old and only demonstrates a fraction of the possibilities you have with the Skyscraper compositor. It's all thoroughly documented below.

### Example artwork.xml

An example could look something like this:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<artwork>
  <output type="screenshot" width="640" height="480">
    <layer resource="screenshot" x="20" width="520" height="390" align="center" valign="middle">
      <rounded radius="10"/>
      <stroke width="5"/>
    </layer>
    <layer resource="cover" height="250" x="0" y="-10" valign="bottom">
      <gamebox side="wheel" rotate="90"/>
      <shadow distance="5" softness="5" opacity="70"/>
    </layer>
    <layer resource="wheel" width="250" x="-10" align="right">
      <shadow distance="5" softness="5" opacity="70"/>
    </layer>
  </output>
</artwork>
```

Which results in a screenshot being exported to look like this:

<figure markdown>
  ![Artwork example](resources/Sonic%20The%20Hedgehog%202.png)
  <figcaption>Artwork example</figcaption>
</figure>

### XML Definitions

Click the following links to quickly go to a desired section.

- Nodes: [XML Preamble](#xml-preamble-m), [Artwork node](#artwork-node-m), [Output node](#output-nodes-o), [Layer node](#layer-nodes-o)
- Effects: [Blur effect](#blur-effect-node-o), [Balance effect](#balance-effect-node-o),
[Brightness effect](#brightness-effect-node-o),
[Color balance effect](#balance-effect-node-o), [Colorize effect](#colorize-effect-node-o),
[Contrast effect](#contrast-effect-node-o), [Frame effect](#frame-effect-node-o),
[Gamebox effect](#gamebox-effect-node-o), [Hue effect](#hue-effect-node-o),
[Mask effect](#mask-effect-node-o), [Opacity effect](#opacity-effect-node-o),
[Rotate effect](#rotate-effect-node-o), [Rounded effect](#rounded-effect-node-o),
[Saturation effect](#saturation-effect-node-o), [Scanlines effect](#scanlines-effect-node-o),
[Shadow effect](#shadow-effect-node-o), [Stroke effect](#stroke-effect-node-o).
- Other: [Custom image resources](#custom-image-resources).

You can also find a XML schema file named `artwork.xsd` sibling to the
`artwork.xml` file in the configuration folder of Skyscraper. The schema file
may come in handy for validation of your artwork file with your favorite XML
editor.

!!! tip "Notation in this guide"

    This guide uses [m] in the headings for any node or node attribute that is
    mandatory in your artwork XML file. An [o] indicates an optional node or
    node attribute.

#### XML Preamble [m]

```xml
<?xml version="1.0" encoding="UTF-8"?>
```

This is a special node used to describe the XML language used in the document. Don't change it.

#### 'artwork' node [m]

```xml
<artwork>
...
</artwork>
```

This node is the document base node. It contains all other nodes. Don't change or delete it.

#### 'output' node(s) [o]

```xml
<output type="screenshot" resource="wheel" mpixels="0.1" width="640" height="400">
...
<output>
```

The output node describes a single piece of artwork to be exported when scraping. You can have as many (or few) output nodes as you like.

An output node with no nested 'layer' nodes will export the type with no compositing.

##### 'type' attribute [m]

The type to be exported. Can be:

-   screenshot
-   cover
-   wheel
-   marquee
-   texture

##### 'resource' attribute [o]

The resource attribute can be set to one of the following:

-   screenshot
-   cover
-   wheel
-   marquee
-   texture

Why type and resource? For instance, if you wish to export a 'marquee' image but want it to make use of the 'wheel' resource instead, you can set `<output type="marquee" resource="wheel"/>`. It will then export a 'marquee' but use the raw unmanipulated 'wheel' image when doing so. So if your frontend theme, such as some EmulationStation themes, makes use of the 'marquee' artwork, it will then be using the 'wheel' for it. If this attribute is left out, 'resource' will be set to the same as 'type'.

!!! info

    The `screenshot`, `cover`, `wheel`, `marquee` and `texture` types are hardcoded. It's not currently possible to create custom types.

##### 'mpixels' attribute [o]

Set the desired area size for the image resource in mega pixels (eg '0.1'). This can be used as an alternative to setting width or height and is useful in cases where images vary a lot in size. 'width' and 'height' attributes will be ignored if this is set.

##### 'width' and 'height' attributes [o]

Sets the width and height of the artwork export. If both are left out it will use the original size of the scraped artwork. If one is left out, it will keep the aspect ratio relative to that.

##### 'aspect' attribute [o]

Can be applied. See description in [layer](#aspect-attribute-o_1)

##### 'transform' attribute [o]

Can be applied. See description in [layer](#transform-attribute-o_1)


#### 'layer' node(s) [o]

```xml
<layer resource="cover" mode="overlay" opacity="50" x="5" y="5"
       mpixels="0.1" width="10" height="10" align="center" valign="middle"/>
...
</layer>
```

Like layers in GIMP or almost any other respectable photo manipulation software, you can add as many of these nodes as you like. You can even nest them inside of each other, in which case the child layer will be anchored to the topleft corner of the parent layer, making it easier to align.

!!! note

    Layer and effect nodes are rendered from top to bottom.

##### 'resource' attribute [o]

The artwork resource to be used by the layer. It can be:

-   screenshot
-   cover
-   wheel
-   marquee
-   texture

If the resource attribute is left out the layer will simply be transparent. This is useful when aligning nested layers that differ in size across games.

##### 'mode' attribute [o]

<figure markdown>
  ![Mode examples](resources/layer_modes.png)
  <figcaption>Mode examples (note the in-image captions or table below)</figcaption>
</figure>

Sets the layer render mode. It can be:

| Fig. Col. 1 | Fig. Col. 2   | Fig. Col. 3 | Fig. Col. 4    | Fig. Col. 5 | Fig. Col. 6     |
| ----------- | ------------- | ----------- | -------------- | ----------- | --------------- |
| multiply    | screen        | overlay     | darken         | lighten     | difference      |
| colordodge  | colorburn     | hardlight   | softlight      | exclusion   | xor             |
| sourcein    | destinationin | sourceout   | destinationout | sourceatop  | destinationatop |

If left out the layer is simply rendered as is on top of the parent layer.

##### 'opacity' attribute [o]

Defines the opacity of the layer. 100 is completely visible. 0 is completely transparent.

##### 'x' and 'y' attributes [o]

X and Y coordinates for placement. Relative to 'align' and 'valign' and the parent layer. Either can be left out in which case it will be set to 0.

##### 'mpixels' attribute [o]

Set the desired area size for the image resource in mega pixels (eg '0.1'). This can be used as an alternative to setting width or height and is useful in cases where images vary a lot in size. 'width' and 'height' attributes will be ignored if this is set.

##### 'width' and 'height' attributes [o]

Sets the width and height of the layer. If either is left out it will keep the aspect ratio relative to the other. If both are left out it will use the original dimensions of the scraped resource.

##### 'aspect' attribute [o]

<figure markdown>
  ![Aspect ratio example](resources/aspectexample.png)
  <figcaption>From left to right: source image, ignore, keep, keepexpand<br>(aspect=... combined with align="center" and valign="middle")</figcaption>
</figure>

When you have set width _and_ height you can control with this attribute how the image is fit into the defined dimensions. Possible values for the attribute are `aspect="ignore"`, `aspect="keep"` or `aspect="keepexpand"` (see figure). If the attribute is absent the default is `"ignore"`.  
Setting this a non default value comes in handy if you have for example a more complex artwork definition and you want the available space best used while maintaining the aspect ratio and don't care about possible blank space (e.g., letterboxing, pillarboxing) as you may have put an extra mask layer. For more information see the [Qt documentation on this topic](https://doc.qt.io/qt-6/qt.html#AspectRatioMode-enum).

This attribute can also be applied on 'mask', 'frame' and 'output' nodes.

##### 'transform' attribute [o]

<figure markdown>
  ![Transform example](resources/transformexample.png)
  <figcaption>Transformation mode: transform="fast" (l.), transform="smooth" (r.) applied to resource="screenshot"<br>(images 200% enlarged from original size)</figcaption>
</figure>

Controls the how the scaling of an image is done. Two values are recognized: Value `"fast"` does not apply an extra processing step after scaling whereas value `"smooth"` does apply bilinear filtering to the scaled image.  
When this attribute is unset, the default is smooth transformation. Setting this attribute to `"fast"` may result in preferrable screenshots when upscaling a small source image from a pixel game (see figure). However, Skyscraper does not deal with potential compression artifacts from the scraping source.

This attribute can also be applied on 'output' nodes.

##### 'align' attribute [o]

The horizontal alignment of the layer. It can be:

-   left
-   center
-   right

The alignment is relative to the parent layer.

##### 'valign' attribute [o]

The vertical alignment of the layer. It can be:

-   top
-   middle
-   bottom

The alignment is relative to the parent layer.

#### 'balance' effect node [o]

<figure markdown>
  ![Effect example](resources/balance.png)
  <figcaption>Balance effect sample</figcaption>
</figure>

```xml
<layer>
  <balance red="10" green="20" blue="30"/>
</layer>
```

Must be nested inside a layer node. Adjusts the color balance of the parent layer.

##### 'red' attribute [o]

The red color adjustment. Can be -255 to 255.

##### 'green' attribute [o]

The green color adjustment. Can be -255 to 255.

##### 'blue' attribute [o]

The blue color adjustment. Can be -255 to 255.

#### 'blur' effect node [o]

<figure markdown>
  ![Effect example](resources/blur.png)
  <figcaption>Blur effect sample</figcaption>
</figure>

```xml
<layer>
  <blur softness="5"/>
</layer>
```

Must be nested inside a layer node. Blurs the parent layer.

##### 'softness' attribute [m]

Defines the radius of the blur. Higher means blurrier.

#### 'brightness' effect node [o]

<figure markdown>
  ![Effect example](resources/brightness.png)
  <figcaption>Brightness effect sample</figcaption>
</figure>

```xml
<layer>
  <brightness value="10"/>
</layer>
```

Must be nested inside a layer node. Adjusts the brightness of the parent layer.

##### 'value' attribute [m]

The difference value for the adjustment. Can be -255 to 255.

#### 'colorize' effect node [o]

<figure markdown>
  ![Effect example](resources/colorize.png)
  <figcaption>Colorize effect sample</figcaption>
</figure>

```xml
<layer>
  <colorize hue="180" saturation="-50"/>
</layer>
```

Must be nested inside a layer node. Colorizes the parent layer with a single hue.

##### 'hue' attribute [m]

Sets the hue in degrees. Can be 0 to 360.

##### 'saturation' attribute [o]

Sets the saturation delta value of the colorize effect. Can be -127 to 127.

#### 'contrast' effect node [o]

<figure markdown>
  ![Effect example](resources/contrast.png)
  <figcaption>Contrast effect sample</figcaption>
</figure>

```xml
<layer>
  <contrast value="10"/>
</layer>
```

Must be nested inside a layer node. Adjusts the contrast of the parent layer.

##### 'value' attribute [m]

The difference value for the adjustment. Can be -255 to 255.

#### 'frame' effect node [o]

<figure markdown>
  ![Effect example](resources/frame.png)
  <figcaption>Frame effect sample</figcaption>
</figure>

```xml
<layer>
  <frame file="[custom image resource]" x="0" y="0" width="100" height="100"/>
</layer>
```

Must be nested inside a layer node. This applies a frame to the parent layer.

!!! tip

    You can get some cool results by first applying a mask, then applying a frame. This is useful for frames that aren't square.

##### 'file' attribute [m]

The filename of the [custom image resource](#custom-image-resources) to be used as frame.

##### 'width' attribute [o]

Sets the width of the frame in pixels. If left out it will be set to the width of the parent layer.

##### 'height' attribute [o]

Sets the height of the frame in pixels. If left out it will be set to the height of the parent layer.

##### 'x' attribute [o]

Sets the x coordinate of the frame relative to the parent layer. If left out it will be set to 0.

##### 'y' attribute [o]

Sets the y coordinate of the frame relative to the parent layer. If left out it will be set to 0.

##### 'aspect' attribute [o]

Can be applied. See description in [layer](#aspect-attribute-o_1)

#### 'gamebox' effect node [o]

<figure markdown>
  ![Effect example](resources/gamebox.png)
  <figcaption>Gamebox effect sample</figcaption>
</figure>

Left image shows the result with the wheel artwork applied to the side. Right image shows the original cover layer.

```xml
<layer resource="cover" height="200">
  <gamebox side="[custom image resource]" rotate="90" sidescaling="both"/>
</layer>
```

Must be nested inside a layer node. Renders a nifty looking 3D game box. It uses the parent layer image on the front of the box.

##### 'side' attribute [o]

The filename of the [custom image resource](#custom-image-resources) to be used on the side of the box.

##### 'rotate' attribute [o]

Defines the rotation of the side image in degrees.

##### 'sidescaling' attribute [o]

Defines how to scale the side / spine image. If left out, it will autoscale depending on the size of the artwork resource. Can be 'width', 'height' or 'both'.

#### 'hue' effect node [o]

<figure markdown>
  ![Effect example](resources/hue.png)
  <figcaption>Hue effect sample</figcaption>
</figure>

```xml
<layer>
  <hue value="100"/>
</layer>
```

Must be nested inside a layer node. Rotates the hue of the parent layer.

##### 'value' attribute [m]

The difference value for the adjustment. Can be 0 to 359.

#### 'mask' effect node [o]

<figure markdown>
  ![Effect example](resources/mask.png)
  <figcaption>Masking sample</figcaption>
</figure>

Left image shows the result. Right image shows the mask used. The white part of the mask is transparent.

```xml
<layer>
  <mask file="[custom image resource]" x="0" y="0" width="100" height="100"/>
</layer>
```

Must be nested inside a layer node. This applies a mask to the parent layer.

##### 'file' attribute [m]

The filename of the [custom image resource](#custom-image-resources) to be used as mask. The alpha channel of the mask will blind out the underlying parts of the parent layer.

##### 'width' attribute [o]

Sets the width of the mask in pixels. If left out it will be set to the width of the parent layer.

##### 'height' attribute [o]

Sets the height of the mask in pixels. If left out it will be set to the height of the parent layer.

##### 'x' attribute [o]

Sets the x coordinate of the mask relative to the parent layer. If left out it will be set to 0.

##### 'y' attribute [o]

Sets the y coordinate of the mask relative to the parent layer. If left out it will be set to 0.

##### 'aspect' attribute [o]

Can be applied. See description in [layer](#aspect-attribute-o_1)

#### 'opacity' effect node [o]

<figure markdown>
  ![Effect example](resources/opacity.png)
  <figcaption>Opacity effect sample</figcaption>
</figure>

```xml
<layer>
  <opacity value="50"/>
</layer>
```

Must be nested inside a layer node. Adjusts the opacity of the parent layer.

##### 'value' attribute [m]

The opacity of the layer. Can be 0-100 where 0 is completely transparent and 100 is opaque.

#### 'rotate' effect node [o]

<figure markdown>
  ![Effect example](resources/rotate.png)
  <figcaption>Rotation examples</figcaption>
</figure>

```xml
<layer>
  <rotate degrees="45" axis="y"/>
</layer>
```

Must be nested inside a layer node. Rotates the parent layer around the x, y or z axis.

##### 'degrees' attribute [m]

Sets how many degrees the parent layer will be rotated. Can be -360 to 360.

##### 'axis' attribute [o]

Sets which axis the parent layer should be rotated around. Can be 'x', 'y' or 'z'. If left out it will be set to 'z'.

#### 'rounded' effect node [o]

<figure markdown>
  ![Effect example](resources/rounded.png)
  <figcaption>Corner rounding sample</figcaption>
</figure>

```xml
<layer>
  <rounded radius="10"/>
</layer>
```

Must be nested inside a layer node. Rounds the corners of the parent layer.

##### 'radius' attribute [m]

Defines the radius of the corners in pixels.

#### 'saturation' effect node [o]

<figure markdown>
  ![Effect example](resources/saturation.png)
  <figcaption>Saturation effect sample</figcaption>
</figure>

```xml
<layer>
  <saturation value="-100"/>
</layer>
```

Must be nested inside a layer node. Adjusts the color saturation of the parent layer.

!!! tip

    If you want to completely greyscale your layer I highly recommend using the 'saturation' attribute of the 'colorize' effect instead. It uses a better estimation of percieved luminance.

##### 'value' attribute [m]

The difference value for the adjustment. Can be -255 to 255.

#### 'scanlines' effect node [o]

<figure markdown>
  ![Effect example](resources/scanlines.png)
  <figcaption>Scanlines effect sample</figcaption>
</figure>

```xml
<layer>
  <scanlines file="scanlines1.png" scale="1.0" opacity="100" mode="overlay"/>
</layer>
```

Must be nested inside a layer node. Adds a CRT monitor scanline effect to the parent layer.

##### 'file' attribute [o]

The filename of the [custom image resource](#custom-image-resources) to be used as the scanline overlay. If left out it defaults to `scanlines1.png`.

##### 'scale' attribute [o]

Sets the scale factor of the overlay. Can be 0.1 to 2.0. If left out it defaults to 1.0.

##### 'opacity' attribute [o]

Sets the opacity of the overlay. Can be 0 to 100. If left out it defaults to 100.

##### 'mode' attribute [o]

Sets the overlay compositing mode. The values are the same as in the [layer 'mode' attribute](#mode-attribute-o).

If left out the overlay is rendered using the `overlay` method.

#### 'shadow' effect node [o]

<figure markdown>
  ![Effect example](resources/shadow.png)
  <figcaption>Dropshadow effect sample</figcaption>
</figure>

```xml
<layer>
  <shadow distance="10" softness="5" opacity="50"/>
</layer>
```

Must be nested inside a layer node. Renders a dropshadow on the parent layer using the attributes provided. If either of the attributes are left out, the shadow won't be drawn.

##### 'distance' attribute [m]

Distance in pixels from the layer. The distance is always down to the right.

##### 'softness' attribute [m]

Defines how soft (radius) the shadow will appear. A value of 0 is sharpest.

##### 'opacity' attribute [m]

Defines the opacity of the shadow. 100 is completely visible. 0 is completely transparent.

#### 'stroke' effect node [o]

<figure markdown>
  ![Effect example](resources/stroke.png)
  <figcaption>Stroke effect sample</figcaption>
</figure>

```xml
<layer>
  <stroke width="5" red="10" green="20" blue="30"/>
</layer>
```

Must be nested inside a layer node. Renders a colored outline on the parent layer. If all color attributes are left out, it averages a suitable color from the parent layer.

##### 'width' attribute [m]

The width of the outline in pixels.

##### 'red' attribute [o]

The red color value for the outline. Can be 0-255. If left out it is set to 0.

##### 'green' attribute [o]

The green color value for the outline. Can be 0-255. If left out it is set to 0.

##### 'blue' attribute [o]

The blue color value for the outline. Can be 0-255. If left out it is set to 0.

##### 'color' attribute [o]

Provides the color to use RGB hexadecimal notation. This can be used instead of the 'red', 'green' and 'blue' attributes described above. An example could be 'color="#ff0099"'. You may also use the [shorthand form](https://en.wikipedia.org/wiki/Web_colors#Shorthand_hexadecimal_form), thus `#f06` will be expanded to `#ff0066`. If the 'color' attribute is provided, then 'red', 'green', 'blue' values are ignored if they are also present.

### Custom image resources

You can also use custom image resources wherever the documentation says so. Place your custom resources in the '`/home/<USER>/.skyscraper/resources`' folder and use it by adding the filename to the attribute.

Example:

```xml
<gamebox side="megadrive_logo.png" rotate="90"/>
```

The file megadrive_logo.png will then be loaded from the '`/home/<USER>/.skyscraper/resources/megadrive_logo.png`' file. Feel free to create subfolders inside the resources folder. If you do, remember to add the partial path to the filename like so:

```xml
<gamebox side="platform_logos/megadrive_logo.png" rotate="90"/>
```

This will look for the file at '`/home/<USER>/.skyscraper/resources/platform_logos/megadrive_logo.png`'.

!!! warning

    All custom image resources are loaded into memory when Skyscraper starts to avoid repeatedly reading them from disc / SD card. So please keep your resources to a minimum - otherwise it might exceed the total memory on the Pi which will cause Skyscraper to get killed by the kernel.
