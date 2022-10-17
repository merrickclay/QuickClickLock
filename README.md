## Quick ClickLock
A simple-to-use tool to toggle QuickLock functionality on Windows with a custom hotkey.

![Quick ClickLock main window](https://i.imgur.com/tCTPlUb.png)

## Features
- Support for user defined hotkey, including modifiers.
- Ability to quickly change the ClickLock activation time to your liking.
- Audio notifications to alert the user when ClickLock has been successfully enabled/disabled.
- Hides in the system tray during normal use to avoid cluttering the taskbar.

## Installation
Download the most recent release: https://github.com/merrickclay/QuickClickLock/releases/tag/v1.0.0

## How to use?
1. Run the .exe file provided in the most recent release.
2. Left click on the green or red mouse icon in your system tray.
3. To set a custom hotkey, left click in the Hotkey input box and enter your hotkey. (Default: Ctrl + Shift + L)
4. To change activation time, click in the Activation Time input box and use the up and down arrow keys to set.
5. To minimize settings window to the system tray, click the 'X' in the top right corner of the window. (Hotkey still works when window is minimized)
6. To close program, right click the green or red mouse icon in the system tray and select 'Exit'.

## Run on Startup
1. Right click the .exe file you downloaded.
2. Click "Create shortcut".
3. Press the **Windows logo key  + R**.
4. Type "shell:startup". The startup folder should open.
5. Copy/paste the shortcut you created into this startup folder.

## Motivation
While playing Foxhole (video game) I found the Windows ClickLock very useful when building or digging trenches. However, not being able to quickly turn off ClickLock before engaging with the enemy in a sudden firefight often proved detrimental to my team. On a few occasions, ClickLock engaging while I was in combat lead to some friendly fire. I thought it would be nice to be able to quickly toggle QuickLock on and off with a custom hotkey, and the idea for Quick ClickLock was born. I had never developed a Windows Desktop App with the Win32 API before, so the opportunity to learn and gain experience from this project was also a key motivating factor.

## Build status
The current build is a beta build. All core functionality has been implemented, but I would like to improve some of the UX before future releases.

## Technologies
Written in C++, using the Win32 API. Developed in Visual Studio 2019.

## Contribute
At this time I am not looking for any contributors to the project.

## Credits
Many thanks to the wonderful documentation being maintained at https://docs.microsoft.com/en-us/windows/win32

## License
MIT License

Copyright (c) 2022 Merrick Clay

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
