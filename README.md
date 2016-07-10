# Notepad-UTF8
Set Notepad's default encoding to UTF-8 from ANSI. 

Written in C99, for Win32 platform.


# Usage
If Notepad-UTF8 is run, every instance of Notepad will use UTF-8 as default encoding.

Precompiled binary and installer is distributed at [project homepage](https://joveler.kr/project/notepad-utf8.html).  
Instead, you can compile and launch `Notepad-UTF8_[Arch].exe` yourself.


# Supported Windows
Notepad-UTF8 supports Windows XP to 10.  

<table>
  <tr>
    <th rowspan="2">Version</th>
    <th colspan="2">Support</th>
    <th colspan="2">Tested<br></th>
  </tr>
  <tr>
    <td>x86</td>
    <td>x64</td>
    <td>x86</td>
    <td>x64<br></td>
  </tr>
  <tr>
    <td>XP<br></td>
    <td>O</td>
    <td>X</td>
    <td>O</td>
    <td>-</td>
  </tr>
  <tr>
    <td>Vista</td>
    <td>O</td>
    <td>O</td>
    <td>X</td>
    <td>X</td>
  </tr>
  <tr>
    <td>7</td>
    <td>O</td>
    <td>O</td>
    <td>O<br></td>
    <td>O</td>
  </tr>
  <tr>
    <td>8</td>
    <td>O</td>
    <td>O</td>
    <td>X</td>
    <td>X</td>
  </tr>
  <tr>
    <td>8.1</td>
    <td>O</td>
    <td>O</td>
    <td>O</td>
    <td>O<br></td>
  </tr>
  <tr>
    <td>10 (Build 10240)</td>
    <td>O</td>
    <td>O</td>
    <td>X</td>
    <td>X</td>
  </tr>
  <tr>
    <td>10 (Build 10586)</td>
    <td>O</td>
    <td>O</td>
    <td>O</td>
    <td>O</td>
  </tr>
</table>

Due to my lack of time, I could not test Notepad-UTF8 in all of supported environments.  
Any contribution including testing on untested environments is welcome.

Notepad-UTF8 needs Notepad's `_g_ftOpenAs` address to work, which means it works with whitelist.  
If you want Notepad-UTF8 to support more versions of Windows, create an issue with necessary resources at GitHub.


# Compile
This program is built under this environment:
- CodeBlocks
- TDM-GCC-64 (based on MinGW-w64)

You can compile BatteryLine yourself using MinGW-w64.


## Using MinGW-w64 and make
1\. Add MinGW-w64 toolchain to PATH. Make sure `gcc, g++, ar, windres, mingw32-make` can be called in console.
2\. Clone [Notepad-UTF8](https://github.com/ied206/Notepad-UTF8).
```
> git clone https://github.com/ied206/Notepad-UTF8.git
> cd Notepad-UTF8
```
3\. Clone and build [minhook](https://github.com/TsudaKageyu/minhook).  
```
Download minhook
> git clone https://github.com/TsudaKageyu/minhook.git
> cd minhook

Build x64 linkable object
> copy build/MinGW/Makefile Makefile
> mingw32-make
> ren libMinHook.a libMinHook_x64.a

Patch Makefile to build x86 version
  add '-m32' argument to Makefile's CFLAGS and LDFLAGS
  add '-F pe-i386' argument to Makefile's WINDRES
  You may refer to this gist : https://gist.github.com/ied206/2bd096a85089e816a897fa4266cf5779

Build x86 linkable object
> mingw32-make clean
> mingw32-make
> ren libMinHook.a libMinHook_x86.a
```
4\. Build project with mingw32-make
```
> cd ..
> mingw32-make
```
5\. x86 and x64 binary is compiled under `bin` folder.


## Microsoft Visual Studio
This code can be compiled under C99 compliance compiler.  
If your version of Visual Studio support C99, this code would be compiled without trouble.  
Any contribution related to add official MSVC support is welcome.


# License
This project is licensed under MIT License.  
This project's api hooking is powered by [minhook](https://github.com/TsudaKageyu/minhook).   
Binary distribution of this project is compiled with TDM-GCC-64, which links [winpthreads](http://mingw-w64.org) provided by MinGW-w64.
