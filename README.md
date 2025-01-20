# ormGenerate

Minimal program to generate ORM (ambient occlusion, roughness, metallicity) textures
as specified in the Khronos documentation: https://www.khronos.org/blog/art-pipeline-for-gltf
The channels are:
Red: AO
Blue: Roughness
Green: Metallicity

Usage:

```
make
./ormGenerate /mnt/c/ambientcg/materials/DiamondPlate007D_2K-JPG/DiamondPlate007D_2K-JPG_Color.jpg
```

The program assumes the folder contains files with the convention:

- `DiamondPlate007D_2K-JPG_Color.jpg` - diffuse, not used by this program but assumed as a base path
- `DiamondPlate007D_2K-JPG_AmbientOcclusion.jpg`
- `DiamondPlate007D_2K-JPG_Roughness.jpg`
- `DiamondPlate007D_2K-JPG_Metalness.jpg`

If one or several files are missing, they are written as zero in the output ORM.
If any file has mismatching size or channel count, the program exits with error.
If any of the input files has over 1 channel, only the red channel is read. Generally
speaking, these files should be single channel grayscale images.
 
The output image will be a 3 channel PNG.
