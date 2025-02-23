# Tiled Importer

Import `.tsx` Tile Set and `.tmx` Tile Map files from Tiled Map Editor into Unreal Engine 5 natively. Necessary Textures and Paper2D Tile Sets and Tile Maps are created automatically during the import process.

## Installation

1. Clone this project or download it and extract the code directly into `<your-project>/Plugins`.
2. Refresh your Visual Studio project.
3. Create a Pipeline asset somewhere in your project for each file type that you need to import:
   - `InterchangeTsxPipeline` for Tile Sets
   - `InterchangeTmxPipeline` for Tile Maps
  
![image](https://github.com/user-attachments/assets/661e666a-821f-49e2-8b0f-7b6ecccd0c96)

3. In your Project Settings, navigate to `Engine` > `Interchange`.
4. In `Content Import Settings` > `Pipeline Stacks` > `Assets` > `Per Translator Pipelines` click `Add Element` for each type of file you need to import.

![image](https://github.com/user-attachments/assets/7c227f3b-944b-4449-9333-e1bece64f6d1)

5. For each element you added:
    - Choose a `Translator`:
      - `InterchangeTsxTranslator` for Tile Sets
      - `InterchangeTmxTranslator` for Tile Maps
    - Add one of the Pipelines you created in step (2)
  
![image](https://github.com/user-attachments/assets/08af0e1f-e1c9-421d-93dc-45e868950893)

That's it! You can now import Tile Map and Tile Set files from Tiled Map Editor by dragging and dropping into the Content Browser or through the Import wizard.

## Limitations

Support is currently limited to specific Tile Map types and settings and Tile Set collision objects.

**Tile Map Types**
- Orthogonal

**Tile Map settings**
- `Right Down` render order

**Tile Set collision objects**
- Rectangle

## Future Development

Future releases will add support for:
- Isometric Tile Maps
- Isometric (Staggered) Tile Maps
- Other render orders
- Polygon and circle collision objects
