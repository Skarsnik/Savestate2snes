# Savestate2snes

Tool to handle savestate with a sd2snes using usb2snes

# Prerequis

You need USB2SNES firmware 6 or later installed on your sd2snes and the desktop application. 
Find them at https://github.com/RedGuyyyy/sd2snes/releases and follow the instructions.

Some free space on your system.

# Usage

Start the Savestate2snes executable. It will ask your to check your prerequis and chose a folder to where to put the Savestates, then create a new game and a category for it. 
Run the game and apply the patch then you are set to start using the savestates system.

The software use your file system as a way to create structures like categories and sub category. 
The savestates are just files, so if you want to move them around you can do it with your files browser.
Be careful Savestate2snes read directories only once, if you move stuff around manually it's better to restart the application.


# Savestates limitations

The savestate patch used is provided by Redguyyyy the author of the usb2snes software and firmware. 
It basicly dump all the different existing memories of your console and put them on a special location.

It does not dump the SRAM, the location where game store your save files, most games does not really need it to work
since it's a slow memory compared to the RAM of the console.

The patch try to restore special registry of the console to have a clear reset when loading a savestate. 
The patch has special code for some games (like super metroid) to restore sound and other special stuff.

If if does not work properly with your game try to contact a romhacker of your game or Redguyyyy to come up with a fix.

# Issues

If you find an issue with Savestate2snes the best way to repport it is to create an issue on the github project page (https://github.com/Skarsnik/Savestate2snes/issues)
You will need a github account for that. You can also directly mail me at skarsnik@nyo.fr

Savestate2snes create a log file with debug information, please attach this file when you encounter the bug (it's cleared at each start of the application).
Type `%appData%/Savestate2snes/ on the file address bar of your file browser to find the log file.



