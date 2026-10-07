Exult for Nintendo 3DS — test build
====================================

NEW 3DS / NEW 2DS XL ONLY. The original 3DS / 2DS does not have the memory
or the CPU for this port; the program checks and refuses to start on one.

SD card layout
--------------
  /3ds/exult/exult.3dsx          the program (launch from the Homebrew Launcher)
  /3ds/exult/blackgate/          your Ultima VII: The Black Gate files
  /3ds/exult/blackgate/static/   (the game's STATIC folder — from GOG or the DOS copy)
  /3ds/exult/exult.cfg           settings (created on first run)
  /3ds/exult/exult_log.txt       log files (send these if something goes wrong)
  /3ds/exult/exult_err.txt

Only the STATIC folder is needed. Saves are written next to it.

Optional (from the free Exult audio pack at exult.info):
  /3ds/exult/data/jmsfx.flx      sound effects (or sqsfxbg.flx)
  /3ds/exult/data/music/*.ogg    recorded soundtrack with ambient music

Saving: Start -> Save Game. "Quick Save" keeps only the current state
(that is what Journey Onward loads); use a numbered slot for a real save.

Screens
-------
  Normal:   game on the TOP screen, a touch keyboard on the BOTTOM screen.
  Swapped:  press SELECT — the game moves to the bottom screen so you can
            tap and drag on it directly with the stylus (tap = click,
            double-tap = use, hold = walk / drag). The top screen shows a
            live copy of the game while you do this. Press SELECT again
            to move the game back up.

Starting a new game: type the name on the touch keyboard (Shift = ^ for
capitals), then tap ENTER three times (name -> sex -> Journey Onward).

Controls
--------
  C-stick        move the mouse pointer on the game screen
  L              left mouse button: tap = look at what is under the pointer,
                 hold + move = pick up / drag
  R              right mouse button: tap = one step toward the pointer,
                 hold = keep walking (farther from you = faster),
                 tap twice = walk to that spot
  A              "do it": use / open / talk to the thing under the pointer,
                 press a button, pick a conversation answer, cast a spell
  B              combat on/off
  X              inventory
  Y              stats
  ZL             eat
  ZR             use keys
  Circle pad     walk
  D-pad          walk one step
  Start          Esc: close the top window, or open the game menu
  Select         swap the game between the two screens
  Touch keyboard F1-F12, letters, numbers, Esc, Shift (^), Space,
                 Backspace (<), Enter
  Home           quit

Touch (when the game is on the bottom screen): tap an object for a small
menu (Use / Look / Get), double-tap = use, press and hold = walk, drag to
move items. Buttons and conversation answers are single taps.

Same idea as the original game's mouse: L is the hand (look, pick up,
use), R is the feet (walk). When the pointer turns into crosshairs
(using one thing on another), press L on the target.
