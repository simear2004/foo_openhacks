### Introduction
OpenHacks is an enhancement component for the foobar2000 player.

Requirements:
- Windows 10 1607+
- foobar2000 v1.5+
- foobar2000 v2.0+ x86/x64 

### Features
- Hide DUI menu bar
- Hide DUI status bar
- Draggable and resizable borderless main window
- ...

### Mod version features:  
1. Added: 3 API interfaces, default is true:  
- EnableWin10Shadow = true; //Enable window shadow under win10, and a border will be generated when the window is active (can be eliminated by js script)  
- DisableResizeWhenMaximized = true; //Disable window resizing when maximizing the window  
- DisableResizeWhenFullscreen = true; //Disable window resizing when full screen  
2. New: double-click the pseudo title bar to maximize it, otherwise restore the window  
3. New: Double-click the pseudo title bar to exit full screen when in full screen mode.  
4. New: Disable the dragging function of the pseudo title bar area when maximizing or full screen  
5. New: During the process of dragging the scroll bar on the edge of the window, if the mouse leaves the window range, the drag operation will still be performed.  
6. Modification: Adjust the response distance of window size sizing to avoid confusion caused by overlapping with narrow scroll bars.  
7. Fix: White background flickering problem at startup  
8. Modification: The component is released in Mod version to distinguish it from the original version of ohyeah boss  

### Thanks to:  
ohyeah (original author)  
dreamawake (foobox theme author test feedback)  

### Download：  
https://github.com/simear2004/foo_openhacks_mod/releases

### Build  
Visual Studio 2022  


