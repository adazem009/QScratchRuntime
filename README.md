# QScratchRuntime
QScratchRuntime is a custom Scratch VM written in C++. It uses Qt's graphics framework for rendering.

The goal is to make Scratch projects, especially those that use pen, faster than in the JavaScript-based VM.

## Work in progress!
This project is in development and it can't run most of Scratch projects. This is the current development status:

### Blocks
- [x] Motion blocks
- [x] Looks blocks
- [x] Sound blocks
- [x] Event blocks
- [ ] Control blocks
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

## Notes
I'm not planning to implement extensions other than Pen. Some features, such as graphics effects may be different than in the original Scratch VM.
