#### WINDOWS ENVIRONMENT VARIABLES TO SET

1. **`COMMONLIB_SSE_FOLDER`**: The path to your clone of Commonlib.
2. **`VCPKG_ROOT`**: The path to your clone of [vcpkg](https://github.com/microsoft/vcpkg).
3. (optional) **`SKYRIM_FOLDER`**: path of your Skyrim Special Edition folder.
4. (optional) **`SKYRIM_MODS_FOLDER`**: path of the folder where your mods are.

#### THINGS TO EDIT

1. In LICENSE:

- **`YEAR`**
- **`YOURNAME`**

2. CMakeLists.txt

- **`AUTHORNAME`**
- **`MDDNAME`**
- (optional) Your plugin version. Default: `0.1.0.0`

3. vcpkg.json

- **`name`**: Your plugin's name.
- **`version-string`**: Your plugin version. Default: `0.1.0.0`

#### FEATURES

Automatically imports:

- [CLibUtil](https://github.com/powerof3/CLibUtil) by powerof3
- [SKSE Menu Framework](https://www.nexusmods.com/skyrimspecialedition/mods/120352) by Thiago099

Template credit:

- [Quantumyilmaz](https://github.com/Quantumyilmaz/SKSE_template)

Hooks credits:
- [doodlum](https://github.com/doodlum/skyrim-poise/blob/main/src/Hooks/HitEventHandler.cpp)
- [colinswrath](https://github.com/colinswrath/BladeAndBlunt/blob/main/include/patches/BashBlockStaminaPatch.h)
- [DTry]()