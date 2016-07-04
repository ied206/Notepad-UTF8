# Notepad-UTF8
Set Notepad's default encoding to UTF-8 from ANSI.
Written in C99, for Win32 platform.

Notepad-UTF8 utilize API Hooking provided by [minhook](https://github.com/TsudaKageyu/minhook).

# Supported Windows
Notepad-UTF8 supports Windows XP to 10.
Notepad-UTF8 needs Notepad's _g_ftOpenAs address to work, which means it works with whitelist.
If you want Notepad-UTF8 to support more versions of Windows, contact me with necessary resources.
Any contribution including testing on untested environments is also welcome.

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
    <td>-</td>
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
    <td>X</td>
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
    <td>X<br></td>
  </tr>
  <tr>
    <td>10 (Build 10240)<br></td>
    <td>O</td>
    <td>O</td>
    <td>X</td>
    <td>X</td>
  </tr>
  <tr>
    <td>10 (Build 10586)<br></td>
    <td>O</td>
    <td>O</td>
    <td>O</td>
    <td>O</td>
  </tr>
</table>

# Usage
Compile and launch `Notepad-UTF8_[Arch].exe` in `bin` folder.

Or, you can download precompiled binary and installer from [project homepage](https://joveler.kr/project/notepad-utf8.html).

# Compile
This program is built under this environment:
- CodeBlocks
- TDM-GCC-64 (based on MinGW-w64)

You can compile BatteryLine yourself using MinGW-w64.

## Using MinGW-w64 and make
1. Add MinGW-w64 toolchain to PATH. Make sure `gcc, g++, ar, windres, mingw32-make` can be called in console.
2. Build project with mingw32-make
3. To compile binary, type:
> mingw32-make
4. x86 and x64 binary is compiled under `bin` folder.

## Microsoft Visual Studio
This code can be compiled under C99 compliance compiler.

If your version of Visual Studio support declaring variable in the middle of code block in C, this code would be compiled without trouble.
Any contribution related to add official MSVC support is welcome.

# License
This project is licensed under MIT License.
This project's api hooking is powered by [minhook](https://github.com/TsudaKageyu/minhook).
Binary distribution of this project is compiled with TDM-GCC-64, which links [winpthreads](http://mingw-w64.org) provided by MinGW-w64.
