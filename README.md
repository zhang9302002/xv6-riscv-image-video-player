## xv6-riscv JPEG/WAV/MP4 player

This project contains multimedia players:
* viewer *.jpeg
* playwav *.wav
* playmp4 *.rgb

This project is based on:
* https://pdos.csail.mit.edu/6.828/2021/index.html
* https://github.com/keshavgupta21/xv6-vga/tree/xv6-riscv-fall19
* http://svn.emphy.de/nanojpeg/trunk
* https://github.com/zhaoyuhang/THSS14-XV6/tree/b3591cd7a69f4725ebf1964888ce86d447135f22/xv6
* 

## Environment Setup

* Install QEMU 5.1+, riscv-tools: 
https://pdos.csail.mit.edu/6.828/2021/tools.html
* Install VNC Viewer to display JPEG pictures:
  https://www.realvnc.com/en/connect/download/viewer
* Install ffmpeg to convert MP4 to rgb+wav:
  http://ffmpeg.org/download.html
* Download this project or Fork it: 
https://github.com/zhang9302002/xv6-riscv
* We test the project on **macOS Monterey 12.0.1**,
and everything goes well

## User Guideline
* to start xv6-riscv64
    > make qemu
* to display jpeg
    > make qemu
  > 
    > viewer hutao.jpeg
  
    to move the picture
    
    > ctrl+z, W/A/S/D
  
    to zoom in / out

    > ctrl+z, O/P

  * to play wav:
    > make qemu
    > 
    > playwav test.wav

* to play mp4:
    > ffmpeg -i a.mp4 -r 10.5 -s 320x200 -pix_fmt rgb565le a.rgb
    >
    > ffmpeg -i a.mp4 -acodec pcm_s161e -ac 2 -ar 22050 a.wav
    >
    > make qemu
    >
    > playmp4 a.rgb

## Note
* The RAM of xv6 is limited to 128MB, so mp4 video
larger than 30s is not supported.
* When you add new files(jpeg, wav or rgb) to xv6, 
you must change [Makefile, L180-181](https://github.com/zhang9302002/xv6-riscv/blob/master/Makefile#L180-L181)
like this:

    > fs.img: mkfs/mkfs *.jpeg *.wav *.rgb user/xargstest.sh &lt;newfile&gt;  $(UPROGS)
    >
    > mkfs/mkfs fs.img *.jpeg *.wav *.rgb user/xargstest.sh &lt;newfile&gt; $(UPROGS)