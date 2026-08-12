# textrend

> A 3D software renderer that renders to the terminal

## 🖥️ Usage

```textrend [filename.obj] [-args]```

This renderer only accepts ```.obj``` files, and you must always pass the file before the arguments. Currently targeting Windows and Linux, although theoretically it should also work on MacOS.

## ⌨️ Controls

| Hotkey | Action |
| --- | --- |
| <kbd>w</kbd><kbd>a</kbd><kbd>s</kbd><kbd>d</kbd> | Move |
| <kbd>o</kbd><kbd>k</kbd><kbd>l</kbd><kbd>;</kbd> or <br><kbd>8</kbd><kbd>4</kbd><kbd>2</kbd><kbd>6</kbd> (numpad) | Look |
| <kbd>\[</kbd> and <kbd>\]</kbd> | +/- movement speed |
| <kbd>\{</kbd> and <kbd>\}</kbd> | +/- rotation speed |
| <kbd>-</kbd> and <kbd>+</kbd> | Zoom in/out |
| <kbd>z</kbd> | Toggle vertices |
| <kbd>x</kbd> | Toggle edges |
| <kbd>c</kbd> | Toggle faces |
| <kbd>b</kbd> | Toggle backface culling |
| <kbd>Esc</kbd> | Exit |

## 🚩 Arguments

| Argument | Description | 
| ------------- | -------------- |
| -v --verbosity | Set verbosity level | 
| -y --flip-y | Flip camera Y rotation | 
| --no-detect-font --no-detect-font-size | (Windows only) Disable auto font size detection; use this if your terminal reports incorrect information | 
| --font --font-ratio | (Windows only) Set fallback font width-to-height ratio | 
| -h --help | Print help message | 

> [!NOTE]
> ```--no-detect-font``` is enabled automatically if your terminal reports a font size that has 0 as one of its dimensions. The majority of modern terminals on Windows report {0, 16} as the size, so the renderer uses the fallback value.

## 🪲 Known issues

- [ ] Sometimes there are holes in between adjacent triangles
- [ ] Adjacent edges of triangles are drawn twice
- [ ] Depth testing suffers from precision errors
