# 3313C_BasicVidTimer
A simple command line script that embeds a timer or caption in a given video. This script invokes ffmpeg.exe and ffprobe.exe, so they are required to be in the same folder as that of the application. 

Make sure that your version of `ffmpeg.exe` is compiled with `--enable-libfreetype` and `--enable-libopenh264`. 
Additionally, you need a C compiler in order to turn `main.c` into `basic_vid_timer.exe`.
If you don't have one, you can download the gcc compiler from here:
  https://gcc.gnu.org/mirrors.html

To compile with gcc, simply type the following:

```bash
  gcc -o basic_vid_timer.exe main.c
```
