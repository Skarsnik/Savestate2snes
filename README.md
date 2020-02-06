# Savestate2snes

A tool to manage savestate with a sd2snes using usb2snes
Learn more at https://skarsnik.github.io/Savestate2snes/

# Prerequis

You need USB2SNES firmware 6 or later installed on your sd2snes and the desktop application.

NEW SD2SNES Firmware will contains the usb2snes feature, be sure to use it instead of USB2SNES firmware when available 

Find them at https://github.com/RedGuyyyy/sd2snes/releases and follow the instructions.

For SNES Classic, you will need it to be patched with Hakchi2 CE
and remove the Clovershell Deamon module.

Some free space on your system.

# Usage

Start the Savestate2snes executable. It will ask your to check your prerequis and chose a folder to where to put the Savestates, then create a new game and a category for it. 
Run the game and apply the patch then you are set to start using the savestates system.


# Quick start

1. Put the savestate2snes folder in QUsb2Snes/apps/
2. Turn on the console and launch QUsb2Snes.
3. Right-click the QUsb2Snes tray icon and click Application -> Savestate2snes
4. Make sure you have the correct version of usb2snes and then check the first launch checkbox. 
Also set your savestate directory (default will be My doc\Savestates). Please note that the directory needs to already exist.
5. Click 'New Game' and then launch the game of your choice on the console.
6. When the game is started and Savestate2snes see it, click "Patch ROM".
7. Right-click the area above "Load Savestate" to create a category. You can also right-click the name of that category
after it's been made to create a sub-category.
8. If the big button is green, it means you're all set! 

To make a save state, either click "Make Savestate" within savestate2snes, this will trigger and save a savestate.
You can also press the defined button combination on your controller and press "Save Savestate" on Savestate2snes to save it.

"Load Savestate" will simply load the selected savestate from the list on the right.

The software uses your file system as a way to create structures like categories and sub-category. 
The savestates are just files, so if you want to move them around you can do it with your file browser. 
Please note that Savestate2snes only read the directories once, so if you move or rename files you'll need to restart the application.

# Savestates limitations

The savestate patch used is provided by Redguyyyy the author of the usb2snes software and firmware. 
It basicly dump all the different existing memories of your console and put them on a special location.

It does not dump the SRAM, the location where game store your save files, most games does not really need it to work
since it's a slow memory compared to the RAM of the console.

The patch try to restore special registry of the console to have a clear reset when loading a savestate. 
The patch has special code for some games (like super metroid) to restore sound and other special stuff.

If it does not work properly with your game try to contact a romhacker of your game or Redguyyyy to come up with a fix.

# SNES Classic

Hack your SNES Classic with hakchi2 CE and remove the Clovershell Deamon module.
You need to install the serverstuff mod, you should find the serverstuff.hmod in Savestate2Snes folder, copy it to the user_mods folders on Hakchi2 CE,
start Hakchi2 and go into the install mod menu
Power on your console
Start Savestate2snes and choose a Savestate directory
Select the SNES Classic mode
Create a new game with the New game button
Right-click on the category view to add a category and click on it (you can do sub categories!)
Run the game you want to use Savestate2snes with
When the init button became available click on it and wait until the game restart
Load/Save/Make Savestate buttons should be available: You are good to go!
You will need to power off/on the console and restart Savestate2snes if you want to change game

# Issues

If you find an issue with Savestate2snes the best way to repport it is to create an issue on the github project page (https://github.com/Skarsnik/Savestate2snes/issues)
You will need a github account for that. You can also directly mail me at skarsnik@nyo.fr JOIN THE LOG FILE

Savestate2snes create a log file with debug information, please attach this file when you repport a bug (it's cleared at each start of the application).
Type `%appData%/Savestate2snes/` on the file address bar of your file browser to find the log file.

# Licence

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
