# PSP Bluetooth Plugin [![C/C++ CI](https://github.com/ste2425/PSP-Bluetooth-Plugin/actions/workflows/c-cpp.yml/badge.svg?branch=develop)](https://github.com/ste2425/PSP-Bluetooth-Plugin/actions/workflows/c-cpp.yml) ![Github latest downloads](https://img.shields.io/github/downloads/ste2425/PSP-Bluetooth-Plugin/total.svg)

Two plugins to provide the use of Bluetooth controllers in PSP games.

## Acknowledgements

The work ive done is just a thin layer ontop of work many other people have done, Iv'e also had lots of help from others.

* TokyoDrift - Originally created work that uses the SIO port to allow external controllers back around 2010. Since uploaded to [Github](https://github.com/unraze/PSXControllerToPSP)
* OPDitto project - A project that uses the SIO port to allow for a real second analog stick. Its SIO implementation has been extremly helpful to me. Repo [Here](https://github.com/Operation-DITTO)
* X41 - Shared with me an initial working SIO hello world app back when i could barely compile PSP code. This was invaluable in helping me get the hardware working right at the start of the project.
* PSP Homebrew Discord - Full of incredibly smart and helpful people too many to list individually. Without their help I'd never have managed to complete any of this.
* Images/Icons from https://icons8.com
* Lusid1 and SolubleShoe for their work on `DayViewr_game` and `Dayviewer` which the overlay plugin is a fork of. Without them i wouldn't have been able to build that.

**Features:**

- Provides controller suport for use in game and the XMB
- Upto 4 controllers (for homebrew that makes use of them)
- Suports rumble (for homebrew that makes use of it)
- Supports LED colour and player LED indicators (for homebrew that makes use of it)
- Optional Overlay plugin to enable pairing and magament of controllers.

---

## Overlay images

⚠️ This feature was 90% AI built, primarily the graphics routines. I can't for the life of me figured out Blitting. Those of you who dislike AI built code this is what you want to look at. ⚠️

Put 32x32 PNG files in `overlay/assets/`. The build converts them at build time and generates C bitmap data.

Draw image with:

```c
DrawImage(&image_name, x, y);
```

---

⚠️ This is very early development. Ive had it working but its unstable.

---