TIMEWAVE - Version 1.2
Greg Kennedy
kennedy.greg@gmail.com
http://nwserver.webhop.org

---
Introduction
---

TIMEWAVE is a 2-d space shooter written in C++ using some of the freely
available SDL libraries.  It runs on both Windows and Linux.

The object of this game is to fight through the mass of enemies in each
level, defeat the level boss, and finally destroy the (well-armed) enemy
space station far from Earth.

It has an interesting "twist" in which you can alter the flow of time.
Build up your time meter by playing the game in fast-forward.  Playing in
slow motion affords you some extra maneuvering room but depletes the meter.

There are six levels in this game:
* Near Earth
* Nebula Region
* Debris Area
* Distant Worlds
* Deep Space
* Null Zone

If you achieve a high score, you can choose a color flag to represent your
ship and have yourself recorded on the high score table.

---
Default controls (keyboard)
---
arrow keys:	move your ship
space:		fire
left ctrl:	"fast forward"
left shift:	"slow motion"

space:		start game
p:		pause
esc:		exit to menu

---
Controls (joystick)
---
directions:	move your ship
Button 1:	"slow motion" / Start
Button 2:	fire
Button 3:	"fast forward"

---
Changes in version 1.2
---
* Reduction in resource file sizes: some items converted to 256-color,
  optipng and audio editing shaved further bytes.
* Renamed hiscore.dat to config.ini, and added more options.
* New command line options: -f (switch to Fullscreen mode), -c (Create Config:
  just write config.ini and exit), -w (switch to Windowed mode)
* Added a "Change Keys" button which allows users to alter the inputs.
* Added "Arcade Mode" switch to the config.ini file.  This causes the game to
  simulate an arcade machine on Free Play: the title screen rotates with the
  high score table.
* The arrow keys now drive the cursor on the high score color picker table.
* Mac OSX port (PPC / Intel Universal Binary, 10.4 or greater).

---
Credits
---

All enemy and player artwork by me (made with Blender), as well as starry
background and menu images.  Space images are lifted from various space news
sites, mostly NASA stock photographs.

Music by chromag, subi, 4-mat, 2pac, and emax.  Sound effects located using
www.findsounds.com or recorded myself.

Timewave is freeware.  Users are encouraged to download, share, and run the
software as distributed - as long as no fee, monetary or otherwise, is charged
for the use or distribution of this software program.

No person shall distribute a modified version of the software or any assets
within.  Email with any questions.

Enjoy!

I distribute the game with all the necessary run-time libraries.  This notice
must be attached to any distribution containing SDL, SDL_mixer or SDL_image:
---

Please include this notice with the runtime environment:

This library is distributed under the terms of the GNU LGPL license:
http://www.gnu.org/copyleft/lesser.html

The source is available from the libraries page at the SDL website:
http://www.libsdl.org/
