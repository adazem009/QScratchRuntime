# QScratchRuntime
QScratchRuntime is a custom Scratch VM written in C++. It uses Qt's graphics framework for rendering.

The goal is to make Scratch projects, especially those that use pen, faster than in the JavaScript-based VM.

## This isn't developed anymore!
### I abandoned this project because things weren't done "properly" here.
### So, I've started with a new project which is much more complete than QScratchRuntime. Make sure to check it out!
### https://github.com/scratchcpp/scratchcpp-player

### Blocks
- [x] Motion blocks
- [x] Looks blocks
- [x] Sound blocks
- [x] Event blocks
- [x] Control blocks
- [ ] Sensing blocks
- [ ] Operator blocks
- [ ] Variables blocks
- [ ] Custom blocks
- [ ] Pen blocks

### Features
- [x] Reporter blocks
- [x] Broadcasts
- [ ] Variables
- [ ] Lists
- [ ] Audio input -
could be implemented using [QAudioInput](https://doc.qt.io/qt-5/qaudioinput.html)
- [ ] Timers
- [ ] Load project from .sb3
- [x] Load project from URL
- [ ] Turbo mode
- [ ] Draggable sprites
- [x] Multithreading (experimental, might break some projects)
- [x] Custom FPS (up to value supported by the display)
- [ ] Scratch 2.0 to 3.0 converter (help needed)
- [ ] Scratch 1.4 and below to 3.0 converter (help needed)

### Graphic effects
- [x] Color
- [ ] Fisheye
- [ ] Whirl
- [ ] Pixelate
- [ ] Mosaic
- [x] Brightness
- [x] Ghost

See the `installGraphicEffects()` function in [src/core/scratchsprite.cpp](https://github.com/adazem009/QScratchRuntime/blob/master/src/core/scratchsprite.cpp)

### Sound effects
- [ ] Pitch
- [ ] Pan left/right

### Extensions
- [ ] Pen
