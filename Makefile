# This makefile is intended to run in Windows
WORKDIR = %cd%

CC = gcc.exe
CXX = g++.exe
AR = ar.exe
LD = g++.exe
WINDRES = windres.exe

EXE_INC = 
EXE_CFLAGS = -Wall -std=c99 
EXE_RESINC = 
EXE_RCFLAGS = 
EXE_LIBDIR = 
EXE_SRCDIR = HookEXE
EXE_LIB = -lversion -lshlwapi -lcomdlg32 -lgdi32
EXE_LDFLAGS = -mwindows

EXE_INC_32 = $(EXE_INC)
EXE_CFLAGS_32 = $(EXE_CFLAGS) -O2 -m32
EXE_RESINC_32 = $(EXE_RESINC)
EXE_RCFLAGS_32 = $(EXE_RCFLAGS) -F pe-i386 
EXE_LIBDIR_32 = $(EXE_LIBDIR)
EXE_LIB_32 = $(EXE_LIB) 
EXE_LDFLAGS_32 = $(EXE_LDFLAGS) -s -m32
EXE_SRCDIR_32 = $(EXE_SRCDIR)
EXE_OBJDIR_32 = .\\obj\\x86\\exe
EXE_DEP_32 = 
EXE_OUT_32 = .\\bin\\Notepad-UTF8_x86.exe

EXE_INC_64 = $(EXE_INC)
EXE_CFLAGS_64 = $(EXE_CFLAGS) -O2 -m64
EXE_RESINC_64 = $(EXE_RESINC)
EXE_RCFLAGS_64 = $(EXE_RCFLAGS) -F pe-x86-64
EXE_LIBDIR_64 = $(EXE_LIBDIR)
EXE_LIB_64 = $(EXE_LIB) 
EXE_LDFLAGS_64 = $(EXE_LDFLAGS) -s -m64
EXE_SRCDIR_64 = $(EXE_SRCDIR)
EXE_OBJDIR_64 = .\\obj\\x64\\exe
EXE_DEP_64 = 
EXE_OUT_64 = .\\bin\\Notepad-UTF8_x64.exe

EXE_OBJ_32 = $(EXE_OBJDIR_32)\\BasicIO.o $(EXE_OBJDIR_32)\\Host.o $(EXE_OBJDIR_32)\\Inject.o $(EXE_OBJDIR_32)\\UI.o $(EXE_OBJDIR_32)\\Main.o $(EXE_OBJDIR_32)\\rc\\HookRes.o
EXE_OBJ_64 = $(EXE_OBJDIR_64)\\BasicIO.o $(EXE_OBJDIR_64)\\Host.o $(EXE_OBJDIR_64)\\Inject.o $(EXE_OBJDIR_64)\\UI.o $(EXE_OBJDIR_64)\\Main.o $(EXE_OBJDIR_64)\\rc\\HookRes.o


all: exe_release_32 exe_release_64 dll_release_32 dll_release_64
clean:
	cmd /c if exist .\\bin rd /s /q .\\bin
	cmd /c if exist .\\obj rd /s /q .\\obj

exe_before_release: 
	cmd /c if not exist .\\bin md .\\bin
	cmd /c if not exist $(EXE_OBJDIR_32) md $(EXE_OBJDIR_32)
	cmd /c if not exist $(EXE_OBJDIR_32)\\rc md $(EXE_OBJDIR_32)\\rc
	cmd /c if not exist $(EXE_OBJDIR_64) md $(EXE_OBJDIR_64)
	cmd /c if not exist $(EXE_OBJDIR_64)\\rc md $(EXE_OBJDIR_64)\\rc

exe_release_32: exe_before_release exe_out_release_32

exe_out_release_32: exe_before_release $(EXE_OBJ_32) $(EXE_DEP_32)
	$(LD) $(EXE_LIBDIR_32) -o $(EXE_OUT_32) $(EXE_OBJ_32)  $(EXE_LDFLAGS_32) $(EXE_LIB_32)
	
$(EXE_OBJDIR_32)\\BasicIO.o: $(EXE_SRCDIR)\\BasicIO.c
	$(CC) $(EXE_CFLAGS_32) $(EXE_INC_32) -c $(EXE_SRCDIR_32)\\BasicIO.c -o $(EXE_OBJDIR_32)\\BasicIO.o

$(EXE_OBJDIR_32)\\Host.o: $(EXE_SRCDIR)\\Host.c
	$(CC) $(EXE_CFLAGS_32) $(EXE_INC_32) -c $(EXE_SRCDIR_32)\\Host.c -o $(EXE_OBJDIR_32)\\Host.o

$(EXE_OBJDIR_32)\\Inject.o: $(EXE_SRCDIR)\\Inject.c
	$(CC) $(EXE_CFLAGS_32) $(EXE_INC_32) -c $(EXE_SRCDIR_32)\\Inject.c -o $(EXE_OBJDIR_32)\\Inject.o

$(EXE_OBJDIR_32)\\UI.o: $(EXE_SRCDIR)\\UI.c
	$(CC) $(EXE_CFLAGS_32) $(EXE_INC_32) -c $(EXE_SRCDIR_32)\\UI.c -o $(EXE_OBJDIR_32)\\UI.o
	
$(EXE_OBJDIR_32)\\Main.o: $(EXE_SRCDIR)\\Main.c
	$(CC) $(EXE_CFLAGS_32) $(EXE_INC_32) -c $(EXE_SRCDIR_32)\\Main.c -o $(EXE_OBJDIR_32)\\Main.o
	
$(EXE_OBJDIR_32)\\rc\\HookRes.o: $(EXE_SRCDIR)\\rc\\HookRes.rc
	$(WINDRES) $(EXE_RCFLAGS_32) -i $(EXE_SRCDIR)\\rc\\HookRes.rc -J rc -o $(EXE_OBJDIR_32)\\rc\\HookRes.o -O coff $(EXE_INC)


exe_release_64: exe_before_release exe_out_release_64
	
exe_out_release_64: exe_before_release $(EXE_OBJ_64) $(EXE_DEP_64)
	$(LD) $(EXE_LIBDIR_64) -o $(EXE_OUT_64) $(EXE_OBJ_64)  $(EXE_LDFLAGS_64) $(EXE_LIB_64)
	
$(EXE_OBJDIR_64)\\BasicIO.o: $(EXE_SRCDIR)\\BasicIO.c
	$(CC) $(EXE_CFLAGS_64) $(EXE_INC_64) -c $(EXE_SRCDIR_64)\\BasicIO.c -o $(EXE_OBJDIR_64)\\BasicIO.o

$(EXE_OBJDIR_64)\\Host.o: $(EXE_SRCDIR)\\Host.c
	$(CC) $(EXE_CFLAGS_64) $(EXE_INC_64) -c $(EXE_SRCDIR_64)\\Host.c -o $(EXE_OBJDIR_64)\\Host.o

$(EXE_OBJDIR_64)\\Inject.o: $(EXE_SRCDIR)\\Inject.c
	$(CC) $(EXE_CFLAGS_64) $(EXE_INC_64) -c $(EXE_SRCDIR_64)\\Inject.c -o $(EXE_OBJDIR_64)\\Inject.o

$(EXE_OBJDIR_64)\\UI.o: $(EXE_SRCDIR)\\UI.c
	$(CC) $(EXE_CFLAGS_64) $(EXE_INC_64) -c $(EXE_SRCDIR_64)\\UI.c -o $(EXE_OBJDIR_64)\\UI.o
	
$(EXE_OBJDIR_64)\\Main.o: $(EXE_SRCDIR)\\Main.c
	$(CC) $(EXE_CFLAGS_64) $(EXE_INC_64) -c $(EXE_SRCDIR_64)\\Main.c -o $(EXE_OBJDIR_64)\\Main.o

$(EXE_OBJDIR_64)\\rc\\HookRes.o: $(EXE_SRCDIR)\\rc\\HookRes.rc
	$(WINDRES) $(EXE_RCFLAGS_64) -i $(EXE_SRCDIR)\\rc\\HookRes.rc -J rc -o $(EXE_OBJDIR_64)\\rc\\HookRes.o -O coff $(EXE_INC)


DLL_INC = -Iminhook\\include
DLL_CFLAGS = -DBUILD_DLL -std=c99 
DLL_RESINC = 
DLL_LIBDIR = 
DLL_SRCDIR = HookDLL
DLL_LIB = -luser32 -lshlwapi -lversion
DLL_LDFLAGS = 

DLL_INC_32 = $(DLL_INC)
DLL_CFLAGS_32 = $(DLL_CFLAGS) -O2 -Wall -m32
DLL_RESINC_32 = $(DLL_RESINC)
DLL_RCFLAGS_32 = $(DLL_RCFLAGS) -F pe-i386
DLL_LIBDIR_32 = $(DLL_LIBDIR)
DLL_LIB_32 = $(DLL_LIB) minhook\\libMinHook_x86.a
DLL_LDFLAGS_32 = $(DLL_LDFLAGS) -s -m32
DLL_OBJDIR_32 = .\\obj\\x86\\dll
DLL_SRCDIR_32 = $(DLL_SRCDIR)
DLL_DEP_32 = 
DLL_DEF_32 = .\\bin\\Notepad-UTF8_x86.def
DLL_SHAREDINT_32 = .\\bin\\libNotepad-UTF8_x86.a
DLL_OUT_32 = .\\bin\\Notepad-UTF8_x86.dll

DLL_INC_64 = $(DLL_INC)
DLL_CFLAGS_64 = $(DLL_CFLAGS) -O2 -Wall -m64
DLL_RESINC_64 = $(DLL_RESINC)
DLL_RCFLAGS_64 = $(DLL_RCFLAGS) -F pe-x86-64
DLL_LIBDIR_64 = $(DLL_LIBDIR)
DLL_LIB_64 = $(DLL_LIB) minhook\\libMinHook_x64.a
DLL_LDFLAGS_64 = $(DLL_LDFLAGS) -s -m64
DLL_OBJDIR_64 = .\\obj\\x64\\dll
DLL_SRCDIR_64 = $(DLL_SRCDIR)
DLL_DEP_64 = 
DLL_DEF_64 = .\\bin\\Notepad-UTF8_x64.def
DLL_SHAREDINT_64 = .\\bin\\libNotepad-UTF8_x64.a
DLL_OUT_64 = .\\bin\\Notepad-UTF8_x64.dll

DLL_OBJ_32 = $(DLL_OBJDIR_32)\\DllMain.o $(DLL_OBJDIR_32)\\rc\\HookRes.o
DLL_OBJ_64 = $(DLL_OBJDIR_64)\\DllMain.o $(DLL_OBJDIR_64)\\rc\\HookRes.o
#DLL_LINK_32 = -Wl,--output-def=$(DLL_DEF_32) -Wl,--out-implib=$(DLL_SHAREDINT_32) -Wl,--dll $(DLL_OBJ_32)
#DLL_LINK_64 = -Wl,--output-def=$(DLL_DEF_64) -Wl,--out-implib=$(DLL_SHAREDINT_64) -Wl,--dll $(DLL_OBJ_64)
DLL_LINK_32 = -Wl,--dll $(DLL_OBJ_32)
DLL_LINK_64 = -Wl,--dll $(DLL_OBJ_64)


dll_before_release: 
	cmd /c if not exist .\\bin md .\\bin
	cmd /c if not exist $(DLL_OBJDIR_32) md $(DLL_OBJDIR_32)
	cmd /c if not exist $(DLL_OBJDIR_32)\\rc md $(DLL_OBJDIR_32)\\rc
	cmd /c if not exist $(DLL_OBJDIR_64) md $(DLL_OBJDIR_64)
	cmd /c if not exist $(DLL_OBJDIR_64)\\rc md $(DLL_OBJDIR_64)\\rc

dll_release_32: dll_before_release dll_out_release_32

dll_out_release_32: dll_before_release $(DLL_OBJ_32) $(DLL_DEP_32)
	$(LD) -shared $(DLL_LIBDIR_32) $(DLL_LINK_32)  -o $(DLL_OUT_32) $(DLL_LDFLAGS_32) $(DLL_LIB_32)

$(DLL_OBJDIR_32)\\DllMain.o: $(DLL_SRCDIR)\\DllMain.c
	$(CC) $(DLL_CFLAGS_32) $(DLL_INC_32) -c $(DLL_SRCDIR)\\DllMain.c -o $(DLL_OBJDIR_32)\\DllMain.o

$(DLL_OBJDIR_32)\\rc\\HookRes.o: $(DLL_SRCDIR)\\rc\\HookRes.rc
	$(WINDRES) $(DLL_RCFLAGS_32) -i $(DLL_SRCDIR)\\rc\\HookRes.rc -J rc -o $(DLL_OBJDIR_32)\\rc\\HookRes.o -O coff $(DLL_INC)

dll_release_64: dll_before_release dll_out_release_64

dll_out_release_64: dll_before_release $(DLL_OBJ_64) $(DLL_DEP_64)
	$(LD) -shared $(DLL_LIBDIR_64) $(DLL_LINK_64)  -o $(DLL_OUT_64) $(DLL_LDFLAGS_64) $(DLL_LIB_64)

$(DLL_OBJDIR_64)\\DllMain.o: $(DLL_SRCDIR)\\DllMain.c
	$(CC) $(DLL_CFLAGS_64) $(DLL_INC_64) -c $(DLL_SRCDIR)\\DllMain.c -o $(DLL_OBJDIR_64)\\DllMain.o

$(DLL_OBJDIR_64)\\rc\\HookRes.o: $(DLL_SRCDIR)\\rc\\HookRes.rc
	$(WINDRES) $(DLL_RCFLAGS_64) -i $(DLL_SRCDIR)\\rc\\HookRes.rc -J rc -o $(DLL_OBJDIR_64)\\rc\\HookRes.o -O coff $(DLL_INC)
	
.PHONY: clean exe_release_32 exe_out_release_32 exe_release_64 exe_out_release_64 dll_release_32 dll_out_release_32 dll_release_64 dll_out_release_64

