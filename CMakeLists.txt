cmake_minimum_required(VERSION 3.20)
project(xv6_riscv)

set(CMAKE_CXX_STANDARD 14)
include_directories("${PROJECT_SOURCE_DIR}")

add_executable(xv6_riscv
        user/cat.c
        user/echo.c
        user/forktest.c
        user/grep.c
        user/hello.c
        user/init.c
        user/kill.c
        user/ln.c
        user/ls.c
        user/mkdir.c
        user/rm.c
        user/sh.c
        user/stressfs.c
        user/usertests.c
        user/user.h
        user/grind.c
        user/wc.c
        user/zombie.c

        user/hello.c
        user/mysh.c
        user/printf.c
        user/pwd.c
        user/top.c
        user/ulib.c
        user/umalloc.c

        user/font.h
        user/viewer.c

        user/playwav.c
        user/touch.c
        user/uptime.c
        user/cp.c
        user/mv.c
        user/editor.c
        user/ren.c
        user/shell_sh.c
        user/decode.c
        user/decode.h

        user/parsemp4.c
        user/playmp4.c

        mkfs/mkfs.c
        kernel/bio.c
        kernel/buf.h
        kernel/buddy.c
        kernel/console.c
        kernel/date.h
        kernel/defs.h
        kernel/elf.h
        kernel/exec.c
        kernel/fcntl.h
        kernel/file.c
        kernel/file.h
        kernel/fs.c
        kernel/fs.h
        kernel/kalloc.c
        kernel/log.c
        kernel/list.c
        kernel/main.c
        kernel/memlayout.h
        kernel/param.h
        kernel/palette.h
        kernel/pci.c
        kernel/pipe.c
        kernel/plic.c
        kernel/printf.c
        kernel/proc.c
        kernel/proc.h
        kernel/ramdisk.c
        kernel/riscv.h
        kernel/sleeplock.c
        kernel/sleeplock.h
        kernel/spinlock.c
        kernel/spinlock.h
        kernel/start.c
        kernel/stat.h
        kernel/string.c
        kernel/syscall.c
        kernel/syscall.h
        kernel/sysfile.c
        kernel/sysproc.c
        kernel/trap.c
        kernel/types.h
        kernel/uart.c

        kernel/vga.h
        kernel/vga.c
        kernel/virtio.h
        kernel/virtio_disk.c
        kernel/vm.c

        kernel/sound.c
        kernel/sound.h
        kernel/sysaudio.c
        )
