# FixSurfaceKeyboard

[-> Download](https://github.com/stephtr/FixSurfaceKeyboard/releases)

On many modern keyboards multimedia and function keys are combined with a seperate Fn key for switching between them. The disadvantage of this practice is that simply pressing combinations like <kbd>Alt</kbd>+<kbd>F4</kbd> is only possible when the seperate Fn key is in the right state (which requires to press for example <kbd>Alt</kbd>+<kbd>Fn</kbd>+<kbd>MEDIA_PREV_TRACK/F4</kbd>) even though pressing <kbd>Alt</kbd> + a multimedia key wouldn't make sense.
Therefore some Dell notebooks automatically mapped such otherwise pointless combinations to their according Fn key complement; a function that I'm missing massively on my Microsoft Surface. The Surface Ergonomic Keyboard (at least with German keyboard layout) goes the extra mile and accepts the Fn key in key combinations only if it is the first key pressed which is very problematic if used to pressing <kbd>Alt</kbd> and <kbd>Fn</kbd> at the same time which results in a 50:50 chance.

The goal of this project is to automatically map those key combinations wherever technically possible. It doesn't work for all function keys since some of them don't send any (simply) observable events (for example "de-/increase keyboard brightness"), while others emulate a whole bunch of keys (for example pressing just the search/F9 key on a Surface Ergonomic Keyboard sends events for <kbd>L_WIN</kbd>+<kbd>L_SHIFT</kbd>+<kbd>F21</kbd> while pressing connect/F11 translates to <kbd>L_WIN</kbd>+<kbd>R_CTRL</kbd>+<kbd>F21</kbd>) and others are not indisputably to map (for example on Type Cover 4 <kbd>Shift</kbd>+<kbd>Pos1</kbd> is a legal key combination and must not be automatically mapped to <kbd>Shift</kbd>+<kbd>F8</kbd>).

### Technical details

A significant problem is that the mapping between function keys and multimedia keys is different across different keyboards. It therefore isn't enough to intercept keyboard events and modify (resend) them using a keyboard hook, one also has to register for Raw Input events (which contain information about the device generating them). However Raw Input events are not sent for all keys and it's furthermore not defined if they get received before or after hook events. For yet supporting scenarios like switching between a Surface Ergonomic Keyboard (bluetooth) and a (recently) attached Type Cover this program internally keeps track of the currently attached keyboards and the keyboard beeing used most recently.

Administrative privileges are not required for running this program, even though they are useful if one wants to for example close any elevated windows using <kbd>Alt</kbd>+<kbd>F4</kbd>). The binary files supplied therefore are:
- a portable executable, which doesn't require administrative privileges but can be started with them (via exe file's context menu);
  The application also has an `/install` and `/uninstall` flag, which (un-)installs a windows task which automatically starts the application elevated.
- an installer which automatically takes care of placing the executable in an appropriate directory, running the `/install` command and providing an uninstaller.

### Support for (additional) keyboards

Since I am limited to just a handful of keyboards this program currently just supports (I am not sure if that support at the moment is limited to specific keyboard layouts):
- Surface Type Cover 3/4/2017
- Surface Book 1
- Surface Ergonomic Keyboard

Adding additional keyboards is appreciated and should be quite simple (see [#2](https://github.com/stephtr/FixSurfaceKeyboard/issues/2)), it also would be nice if someone could verify if the program currently works with one of the keyboards mentioned above running another (for example English) keyboard layout.
