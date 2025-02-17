# flutter-windows-ANGLE-OpenGL-Direct3D-Interop
Hardware accelerated rendering on Flutter Windows Texture Widget using OpenGL ES 2.0 from ANGLE. 

<p align='center'><img src='https://user-images.githubusercontent.com/28951144/177827046-35d2599e-6162-49a0-989f-048dc3b40bb5.png'></img></p>

## Introduction

The main goal is to render stuff inside Flutter Windows using OpenGL APIs in a hardware-accelerated manner with GPU-backed buffers. This in turn can be used to render hardware accelerated video-playback, a game's visual output etc.

[ANGLE (Almost Native Graphics Layer Engine)](https://github.com/google/angle) is used for this purpose, which translates these OpenGL calls to Direct3D (which Flutter Windows now supports) calls internally & performs rendering.

The example uses the [new Direct3D texture interop capability for Flutter Windows](https://github.com/flutter/engine/pull/26840) added by [@jnschulze](https://github.com/jnschulze). I compiled ANGLE for Windows on my machine & [.DLLs / .LIBs are present in this repository](https://github.com/alexmercerind/flutter-windows-ANGLE-OpenGL-Direct3D-Interop/tree/master/windows/bin), which are used by the application. The code in this repository is very straightforward & procedurally written without any boilerplate. Hope this serves as a great example.

## Notes

As of 07/07/2022, you need to be on `master` channel of Flutter.

```bash
Flutter 3.1.0-0.0.pre.1533 • channel master • https://github.com/flutter/flutter.git
Framework • revision 78e3b93664 (5 hours ago) • 2022-07-07 08:34:06 -0400
Engine • revision 56faff459e
Tools • Dart 2.18.0 (build 2.18.0-261.0.dev) • DevTools 2.15.0
```

**For Running:**

```bash
git clone https://github.com/alexmercerind/flutter-windows-ANGLE-OpenGL-Direct3D-Interop.git
cd flutter-windows-ANGLE-OpenGL-Direct3D-Interop
cd example
flutter run --verbose
```

## Acknowlegements

- [@jnschulze](https://github.com/jnschulze) for working on [Windows: Add Direct3D texture interoperability support](https://github.com/flutter/engine/pull/26840) in [flutter/engine](https://github.com/flutter/engine). 
- [@clarkezone](https://github.com/clarkezone) for awesome ANGLE + Windows article at [A common OpenGL renderer for UWP and Win32 using ANGLE and Win.UI.Composition](https://clarkezone.github.io/angle/2020/02/24/angle.html). 
## References  

- [Example project showing usage of ANGLE on Windows from @clarkezone](https://github.com/clarkezone/anglehosting).
- [Interop-with-other-DirectX-code section in Microsoft's ANGLE repository](https://github.com/Microsoft/angle/wiki/Interop-with-other-DirectX-code).
- [HelloTriangle.cpp from ANGLE repository](https://github.com/google/angle/blob/main/samples/hello_triangle/HelloTriangle.cpp).

## License 

The source code in this repository is under [The Unlicense](https://unlicense.org/) license.
