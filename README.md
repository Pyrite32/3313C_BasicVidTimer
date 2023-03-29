# 3313C_BasicVidTimer
A simple command line script that embeds a timer or caption in a given video. I wrote this script to embed text and timecodes in my DIG3313C video submissions. This script invokes ffmpeg.exe and ffprobe.exe, so they are required to be in the same folder as that of the `basic_vid_timer.exe`. 

Make sure that your version of `ffmpeg.exe` is compiled with `--enable-libfreetype` and `--enable-libopenh264`. 
Additionally, you need a C compiler in order to turn `main.c` into `basic_vid_timer.exe`.
If you don't have one, you can download the gcc compiler from here:
  https://gcc.gnu.org/mirrors.html

To compile with gcc, simply type the following:

```bash
  gcc -o basic_vid_timer.exe main.c
```

To print a caption to the screen, type: 
```bash
  .\basic_vid_timer.exe -i input.mp4 -text "Hello, World!" -o output.mov
```

To enable timecodes at the center of the screen, type:
```bash
  .\basic_vid_timer.exe -i input.mp4 -USE_TIMER=TRUE -o output.mov
```

For further assistance with syntax, type:
```bash
  .\basic_vid_timer.exe
```
