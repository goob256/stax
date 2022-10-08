----------------
Stax 1.38 README
----------------

Stax is a free falling blocks game for Linux and Windows. It is licensed under
the MIT license. See the file "LICENSE.txt" for details on the license.

Stax was written by Trent Gamblin. The music is by Andrew Skrypnyk.

And, for suggested improvents and help debugging, thanks to:
  Jon Rafkind
  Miran Amon
  Andrew Skrypnyk

----------
Installing
----------

All you need is the executable, named stax or stax.exe. Place it in your path
and you're done.

--------
Controls
--------

Controls for Stax vary from game to game and depend on how you configure the
controls via the in-game menus. Below is a description of the controls
available in each game. If you are using the "W, A, S, D" controls, substitute
"Up" with "W", "Left" with "A", "Right" with "D", and "Down" with "S".
"Button" is either Control, Space or a joystick button, depending on your
configuration.

  ------
  Sucker
  ------
  Left/Right - Move the magnet to the left or right
  Up - Activate the magnet, sucking the top block under the magnet up
  Down - Release a block that has been sucked all the way up

  ----------
  SpringShot
  ----------
  Left/Right - Move the spring to the left or right
  Up - Recoil the spring -- each step will destroy one block upon release
  Down - Release the block

  ------
  Shifty
  ------
  Left/Right - Move left or right
  Up/Down - Move up or down
  Button - Swap blocks

------------
Command Line
------------

Several options can be set from the command line. Here is the list:

  -windowed   -- Run in windowed mode
  -fullscreen -- Run in fullscreen mode
  -sw ###     -- Screen width
  -sh ###     -- Screen height
  -bpp ##     -- Color depth
  -blocks #   -- Number of block types
  -height ##  -- Initial height of blocks

------
DIGMID
------

If MIDI is not supported by your sound card, you can use the DIGMID driver.
To use the DIGMID driver for music, you need a patch set. You can download one
from:
http://www.eglebbk.dds.nl/program/download/digmid.dat
Rename it patches.dat and put it in the same directory as the stax executable
to use it. Thanks to Evert Glebbeek for creating this datafile from the
EAWPATS.
