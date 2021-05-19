It's possible to create a better export by editing these files:


header_insert.txt - added to the index.html first
footer_insert.txt - added to the index.html last
export_view.css - The .css file the default header_insert.txt references

text_overlay_template.txt - this html is added for each text overlay. Certain keywords will
be replaced by data:

[X] = top left X coordinate in pixels where the text overlay should start
[Y] = topleft left Y coordinate in pixels where the text overlay should start
[END_X] = bottom right X coordinate in pixels where the text overlay should end
[END_Y] = bottom right Y coordinate in pixels where the text overlay should end
[WIDTH] = Width in pixels this text block should take
[HEIGHT] = Height in pixels this text block should take
[FONT_SIZE] = Html font size the text should be as guessed by UGT
[TEXT] = Original untranslated text (will use <BR> instead of linefeeds)
[TRANSLATED_TEXT] = Translated text, if it exists.  (will use <BR> instead of linefeeds, but probably won't have linefeeds so text wrapping is needed)

Seth sucks at html so if you know how to add a button to toggle between translated/untranslated text
that'd be like, great.