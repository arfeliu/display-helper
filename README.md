# Display-Helper

Lightweight Windows tray app to quickly adjust your monitor settings

## Project Structure

```
display-helper/
├─ src/
│ └─ main.cpp
├─ resources/icons/
│ └─ brightness.ico
├─ resource.h
├─ resource.rc
└─ build/ ← compiled outputs will go here
```

---

## Build Instructions (Windows 11, MSVC)

### Step 1 — Create the build folder

```x64 Native Tools Command Prompt for VS 2022
mkdir build
```

### Step 2 — Compile the resource file

```x64 Native Tools Command Prompt for VS 2022
rc /I . /fo build\resource.res resource.rc
```

- /I . → include current folder so resource.h can be found
- /fo build\resource.res → output .res file in build/

### Step 3 — Compile the C++ source file

```x64 Native Tools Command Prompt for VS 2022
cl /I . /c src\main.cpp /Fobuild\main.obj
```

- /c → compile only, do not link
- /Fobuild\main.obj → output object file into build/

### Step 4 — Link everything into an executable

```x64 Native Tools Command Prompt for VS 2022
link build\main.obj build\resource.res user32.lib shell32.lib dxva2.lib /OUT:build\main.exe
```

- /OUT: → specify output executable path
- Required libraries:
  - user32.lib → Windows GUI
  - shell32.lib → Shell functions
  - dxva2.lib → Monitor brightness API
