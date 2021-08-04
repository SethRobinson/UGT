Universal Game Translator

It's a pretty jank translation utility designed to translate text on your desktop, or from a live camera feed.

Features:

 * Does optical character recognition (Images to text)
 * Does text translation (from and to most languages)
 * Does text to speech (can hear the from or to language being spoken)
 * Can press E to export to a webpage (Useful for language study, for example, using Rikichan on kanji)
 * Will display a notification if a new version is detected on startup (can be disabled in config.txt)
 * Press D or L to toggle forcing dialog or line-by-line translation modes instead of auto.
 * Ctrl-F10 to drag a rect, this size will be remembered for future snap and translate clicks.

Uses Google's Vision API under the hood for the OCR, and either Google or DeepL cloud services for the translation.  (or both, you can toggle between, just depends what you've setup in the config.txt)

Desktop mode - Useful for games running in windows, webpages, or pictures. It can be controlled
by a gamepad as well.  Some gamepads (XBOX pads/etc) can be used to initiate a capture, even
while using the same pad to play the game.  (By default, clicking down on the right stick will do it)

Camera mode - Snaps pictures from an HDMI input.  Tested with
Elgato Game Capture and Elgato Cam-Link. 

Both modes can optionally be controlled with a controller, works well with a Xbox 360 pad. 

In desktop mode, the "snap and translate" button is the right stick (pressed inward, so it clicks) by default.  It's possible to play a game using the same controller, as long as the controller supports XInput.

In Camera mode, the default button is B.

Before using this, at a minimum, you need to setup your Google account.  Edit the file config_template.txt 
for more directions.  (You'll need to rename it config.txt as well)

For more info and help, visit the blog post about this:

https://www.codedojo.com/?p=2426

This is a free, open source project. 
github (a good place to ask questions or report bugs): https://github.com/SethRobinson/UGT

Seth A. Robinson
www.rtsoft.com