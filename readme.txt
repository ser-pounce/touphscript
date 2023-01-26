touphScript v1.5.0 - a FFVII-PC text editor

Use at your own risk!

Features:
	- Dumps and re-encodes FFVII text using simple UTF-8 text files.
	- All text in the game is fully supported (see files supported).
	- Automatically resizes field and world map windows to fit text.
	- Optionally inserts missing windows in field script.
	- Unused text & scripts optionally removed.
	- Options for controlling resizing, such as character name width.
	- Window values, numeric displays and question parameters can be specified
		 manually if necessary.
	- Tutorial script can be edited.
	- Entries can be ignored in order to selectively overwrite different parts
		 of the text.
	- Supports the English Steam version of Final Fantasy VII

Files supported:
	- ff7.exe / ff7_en.exe (Various menu text and Characters Lv4 Limit dialogue)
	- flevel.lgp (Field and tutorial dialogue)
	- KERNEL.BIN (Sephiroth, Ex-SOLDIER, Red XIII text and scene lookup table)
	- kernel2.bin (Most of game items and their descriptions)
	- scene.bin (Battle dialogue, enemy names, enemy attack names)
	- world_us.lgp (World map dialogue)

Contents:
	- touphScript.exe
	- touphScript.ini
	- touphScript.xml (notepad++ syntax highlighter)
	- chars.txt
	- zlib1.dll
	- Blank (folder holds a blank template of FF7.txt, Kernel2.txt and Scene.txt)
	- readme.txt

Usage:
	1. Double click to start, or run in shell, press d [enter] to dump.
	2. Edit text files.
	3. Run again, this time with e [enter].

	Paths are read from .ini, then registry, then local directory - in that order.

Command line usage:
	touphScript.exe [d|e]
	 d dump
	 e encode

-----------------------------------------------------------------
Notes:

	Backup flevel.lgp, world_us.lgp, scene.bin, kernel.bin kernel2.bin
	and ff7.exe before doing anything, and make sure you have write permissions.

	DO NOT change the text file names, they are used to determine which script to
	read from the archives.

	The three tutorials have the suffix .tut.txt.
	World text is dumped to 0_mes.txt,battle text to 0_scene.bin.txt, kernel to 0_kernel.bin.txt, kernel2 to
	0_kernel2.bin.txt and ff7.exe to 0_ff7.exe.txt. The 0_ is for sorting non-field files to the top of the text folder.

Ini file:

	Set options as described in the ini. If you use either the strip text or strip
	scripts options, be sure to use the same values for decode and encode, otherwise
	you will probably have issues	with flevel.lgp.

The script:

	Text files are UTF-8 (BOM optional), make sure you set your text editor
	accordingly if it doesnt auto-detect.

	Field & world text entries, tutorial sections, scene files and kernel sections
	are separated by a single line of ---------- at least 20 chars long.

	You MUST ensure to keep the correct number of entries, this includes seemingly
	empty entries and newline spacing after {NEW}; do NOT delete any entries
	outright (text can be ignored - see Ignoring entries below).

	The first entry in field files is usually the field name (unless there isnt one),
	occasionally there may be more than one. Some names have garbled or otherwise
	incorrect text (see issues), these dont matter as they only occur in fields where
	the menu is	disabled.

	Whitespace (space, tab and newline) is copied 1:1, full character support as
	detailed on the Qhimm wiki:
		http://wiki.qhimm.com/FF7/Text_encoding

	Use \{ and \} for literal curly braces, \\ for backslash and \# for hash.

	For dialogue windows, the FFVII standard is to put the character name on the
	first line (if there is one), and enclose dialogue in double quotes (“ ” for
	English; other quotes available for other languages in the character map).
	Inanimate objects (computers etc.) tend to use " " instead. Dialogue is
	indented with a tab except the first line of dialogue for each new window.
	Non-dialogue tends not to be indented.

Field:

	Name variables are specified with the name in curly braces:
	{CLOUD} {BARRET} {TIFA} {AERITH} {RED XIII} {YUFFIE} {CAIT SITH} {VINCENT} {CID}
	{PARTY #1} {PARTY #2} {PARTY #3}

	Text can be decorated by preceding with the following:
	{GRAY} {BLUE} {RED} {PURPLE} {GREEN} {CYAN} {YELLOW} {WHITE} {FLASH} {RAINBOW}
	FLASH and RAINBOW require identical closing tags (i.e. closing with a color does
	not work) if non-decorated text follows.

	{CHOICE} is a long indent (default 10 spaces) used to make space for the finger glyph.
	Occasionally a double tab is seen instead, presumably a mistake, this is fixed
	automatically, although some windows possibly use tabs for padding text and may
	need to be reset.

	{OK} waits for player confirmation before continuing.

	{NEW} waits for player confirmation, then clears the window and writes from the
	top. MUST be followed eventually by a newline, anything between the end of the
	tag and the newline is ignored (can be used for notes).

	{WAIT x} delays output by x frames (30 frames = 1 second), valid range 0-65535.

	{MAX} Forces maximum character spacing until the next {MAX} tag.

	{MEM1} {MEM2} {MEM3 x x} insert text from memory, these can be shuffled around
	within the same entry, but do not change the values for {MEM3} unless you
	absolutely know what you are doing. {MEM1}/{MEM2} have  their values set by the
	script beforehand, {MEM3} copies data directly from memory - the first value is
	the offset, and the second the length of the string.

	#[x|y|c|w|h|l|t] can be used at the start of an entry to force window position and
	size (e.g. #xwlt 10 150 75 1). l and t are for setting the position of the numeric
	display (clock or numeric) when available, l is the offset from the left edge, and
	t the top.

	Valid	ranges (out of range values are silently ignored):
	 x / s: 0 - 304
	 y: 0 - 224
	 w: 16 - 304
	 h / t: 1 - 13
	
	c can be used instead of x with no value to automatically center the x position on 
	the screen.

	w and h parameters if present will override auto-size; the game
	automatically attempts to shift windows that would end up off-screen.

	#o is used to specify which lines are the first and last options in a question
	window; the first line, or the first line after the last {NEW} tag, is line 0. Do
	not change the number of answers (i.e. l minus f) unless you have modified the
	script. Only one #o should be specified for any given option window in the field,
	world map parameters appear on a single line (i.e. #xywho)

	Multiple window parameters that are dumped can be removed if individual values
	are not needed, e.g.

	 #xywh 84 0 127 3
	 #xywh 84 0 127 3
	 #xywh 84 0 127 3
	 Princess
		“Please help me…
		legendary hero!”

	can be reduced to

	 #xywh 84 0 127 3
	 Princess
		“Please help me…
		legendary hero!”

	if a single set suffices for all windows using this particular text entry;
	note that there are a few cases where the separate parameters are useful (e.g.
	convil_1).

	Numeric windows are auto-sized if no text is in the same window. If there is text,
	the window parameters are ALWAYS dumped, and MUST be present during encoding.

	Questions can be tricky, as some share the same variable, and others can be present
	in scripts that are called multiple times (e.g. gaiin_5 and junonl1). If you are
	changing the line numbers of any questions make sure to check them.

	World map text lacks a few characters, so far tab and {CHOICE} are confirmed
	unavailable (they are field-only functions).

Auto-size:

	Nearly all windows with text can be auto-sized. Auto-sizing is achieved by reading
	the character spacing values inside WINDOW.BIN and ff7.exe. Width is determined
	by the longest line, and height by the largest line count in between individual
	{NEW} tags:

		{BARRETT}
		“Good luck {CLOUD}!
		If you make it, well follow you!”{NEW}
			“Whoa, Ill hold the PHS for you.
			Itll break if it gets wet.”

	has 3 lines until the {NEW} tag, and then another 2, so the window height will
	be 3 lines.

	Use the ini if you want to specify a precise character name width for window size
	calculations, default is OOOOOOOOO (O being the widest standard character).

	If a numeric display is present the window will be resized if necessary, although
	it is probably better to set these manually.

Ignoring entries:

	If an entry in a text file is blank, the entry is ignored during encoding, this is
	to allow mods to specify exactly what needs to be changed and leave other text in
	place.

Tutorials:

	The tutorial scripting can be modified, available button commands are:
	{UP} {DOWN} {LEFT} {RIGHT} {MENU} {CANCEL} {SWITCH} {CONFIRM} {SCROLLDOWN} {TARGET} {SCROLLUP} {CAMERA} 	{PAUSE} {ASSIST}

	{WAIT x} delays output by x frames (30 frames = 1 second), valid range 0-65535.

	{WINDOW x y} sets the window origin for all windows that follow.

	All commands and text must start on a new line.

	If you are modifying the tutorials you may need to set the menu beforehand in
	the field script, have a look at the relevant script with Makou Reactor to get
	an idea of what needs to be done.

scene.bin:

	scene.bin is composed of 256 separate files, for simplicity all of their text is
	dumped into a single file named 0_scene.bin.txt

	Each entry has a header (\\ File x) and files are separated by -----------------.
	The structure is as follows:

		\\ File x
		3 enemy names.
		(newline)
		32 enemy move names.
	If AI text is present:
		(newline)
		Formation & enemy AI text.

	It is imperative that no lines are added or removed anywhere in the text file
	(including blanks), should this happen, scene.bin will not be updated.

	The only variable used is {NAME x} where x is an int used to refer to a
	character name previously pushed in the script, do not change x or add
	{NAME}s in entries that do not feature them unless you understand the
	implications.

	The scene lookup table in kernel.bin is automatically updated.

KERNEL.BIN:

	KERNEL.BIN holds a paltry 2 entries that are actually used in the pc version
	of FFVII: Ex-SOLDIER (Clouds temp name used at the very beginning of the game),
	and Sephiroth (menu & battle); changing either only affects new games.

kernel2.bin:

	kernel2.bin holds most of the menu-related text. Much like scene.bin, you must
	NOT under any circumstances change the number of lines in the entire file, if
	you do the game will likely crash. The ----------------- separators must also be
	left alone.

	Each line holds a single entry, and the following variables are available:
	{CHAR} {ITEM} {NUM} {TARGET} {ATTACK} {ID} {ELEMENT}
	Do not add vars to entries which do not have them unless you know what you are
	doing (e.g. modifying with Proud Clod etc.).

	The window color can be changed to red with the {RED} tag.
	
	The alphabetical item order table in ff7.exe is updated automatically.

ff7.exe:

	ff7.exe holds all other text. Much like the last three files, do not add or
	delete lines if you want this to work properly. Unused text is not dumped,
	for reference use DLPBs exe offset and information table from The Reunion Database:
	https://docs.google.com/spreadsheets/d/1DUjmyW94zcYoX7gIW5yAT4giTPeNmCR4TCaHAiXOLlU/

------------------------------------------------------------------
Known issues:

	A few of the fields set the field name to a duplicated dialogue string, namely:
	itown2, mtcrl_8 and zmind3 (presumably Square failed to update them after they
	were set as menu disabled scenes), only the second entry is viewable in-game.

	It is possible that some windows are still not resized, please let me know
	if you come across any.

	Read the issues regarding numeric windows and question line number modification.

------------------------------------------------------------------
Source:

	Exe compiled with MinGW 4.8.1 on Win8 x64. Should compile on other Windows
	versions, and possibly on other OSs although a few functions in touphScript.exe.cpp
	that use winapi need to be changed, and the resource file cant be used (check Makefile).

------------------------------------------------------------------
Version info:
	1.5.0
		-Removed hard coded support for DLPB's "The Reunion", since that framework now uses external touphScript-format text files.

	1.4.0	- Undocumented update

	1.3.1
		-Added support for DLPBs "The Reunion" R04
		-Playstation icons supported (used in DLPBs "The Reunion")
		-Optional script fixes have been removed.
		-English Steam game support

	1.3.0
		-Added missing snowboard text entry
		-Added support for 2012 flevel
		-Disabled log popup
		-Numeric window fixes
		-Added window centering option
	1.2.9
		- Bugfixes
		- New patch for fship_4, fixes menu access scene bug

	1.2.8
		- Added patch for frcyo (Chocobo farm) to work around question issue.
		- Small change for variable spacing.

	1.2.7
		- Item sorting table in exe updated automatically based on kernel2 text.
		
	1.2.6
		- Another kernel2 bug fix.
		
	1.2.5
		- Bug-fix for kernel2 dumping.
		
	1.2.4
		- Fix for Italian tutorial junpb.tut.
		- Fixes for swapped offsets in Italian flevel.
		- Fix for dumping buggy Italian kernel2.bin
		
	1.2.3
		- Fix for delimiter issue in Spanish flevel.
		- Fix for Spanish corrupted field shpin_22
		- Fix for German tutorial bug
		
	1.2.2
		- Fixes two pointer bugs with original Spanish flevel
		
	1.2.1
		- Added more script changes.
		- exe string length check & updated length size.
		
	1.2
		- Rehashed question var handling again.
		- Script patching added.
		- Spacing fix.
		
	1.1.5
		- Question parameter vars fixes.
		- Fixed scene.bin jumps when changing text size.
		
	1.1.4
		- Long jump opcode bug-fix.

	1.1.3
		- Bug-fix for mysterious unused scripts.
		
	1.1.2
		- Bug-fix for UTF-8 handling.
		- Fixed missing menu mod character.
	
	1.1.1
		- Removed buggy support for adding text.
		
	1.1
		- Bug-fixes.
		- Window parameter layout change.
		- Better auto-sizing for variables.
		- Numeric displays auto-positioned and positioned correctly if forced.

	1.0
		- Code rewrite
		- All window related params now support forcing values.
		- Multiple windows are dumped as separate params.
		- Auto-sizing now based on values inside window.bin.
		- Some options removed as unnecessary.

	0.9.9.4
		- Bug-fix for world map text boxes and forced parameters.
		- Special windows (clock / numeric) are now resized.

	0.9.9.3
		- Fix for mismatched text entries.

	0.9.9.2
		- Added a check for non-existent script entities.

	0.9.9.1
		- Fixed backslash escape char for literal curly braces etc.
		- Any superfluous data at the end of kernel2.bin is ignored.

	0.9.9
		- Individual entries can be ignored during encoding by replacing entry
		   with single newline.
		- Renamed non-flevel text files to sort to top of folder.

	0.9.8
		- Added support for ff7.exe and kernel.bin text.

	0.9.6
		- Fixed bug in scene.bin for handling jumps.

	0.9.5
		- Added support for scene.bin battle text, table in kernel.bin also.
		   automatically updated.
		- Added support for kernel2.bin.

	0.9
		- Data dir now read from registry if not set in .ini, falls back to local dir.
		- Window values dumped / encoded with text only if a window exists.
		- Tutorials fully editable.
		- Various small background improvements.

	0.8
		- Missing windows now added automatically (experimental).
		- Window parameters can be dumped from text.
		- Override individual window parameters from text.
		- .lgp files now processed as temp files to avoid corrupting the originals.

	0.7
		- Window parameters can be overridden.

	0.6.1
		- Bug-fixes.

	0.6
		- Code rewritten (again), now in c++ and exe is properly optimized.
		- Bug fixes.
		- Some ini options removed for simplicity.
		- Proper error checking and log file added.

	0.5.1
		- Bug fixes.
		- World map windows now resized.

	0.5
		- Code rewrite, now operates entirely in memory with no tempfiles / file cache.
		- Ini file to configure support for DLPBs menu mod, ligatures, character name
		   spacing and tab/space conversion, and file paths.
		- Text can be added at the end of files if necessary.
		- A few bug-fixes.
		- Some function changes (E2 is now MEM3 etc.).
		- Tutorials now dumped to main field text folder, editable.
		- World text can be edited.
		- Fancy icon courtesy of DLPB.

	0.4
		- Full char-map now supported (except {).
		- bug-fix for {PAUSE} and {E2}.
		- code slightly reworked.

	0.3
		- Text files have .txt extension.
		- Dumps tutorial text in main folder (rebuilding tutorial files to come).
		- Auto-size (may not work correctly in places, needs testing!).
		- Misc. bug fixes.

	0.2
		- Fixed spacing related to commas followed by a space.
		- Streamlined dumping and re-encoding, single .exe
		- Field names now always at the top of text files.
		- Creates fields.txt which contains a field name to file map.
		- DD now displays as PAUSE.

	0.1
		- First release.

------------------------------------------------------------------
Credits:

	Many thanks to DLPB for endless assistance, testing, and suggestions.

	Thanks to the Qhimm forum & wiki for invaluable info
		http://forums.qhimm.com/index.php
		http://wiki.qhimm.com/Main_Page

	DLPBs menu and translation projects
		http://forums.qhimm.com/index.php?topic=11649.0
		http://forums.qhimm.com/index.php?topic=11867.0

	Aali for lgp tools and other essential info
		http://forums.qhimm.com/index.php?topic=8641.0

	myst6res Makou Reactor & technical info
		http://forums.qhimm.com/index.php?topic=9658.0
		http://www.wikisquare.com/ff7/technique

	Squall78s Loveless
		http://forums.qhimm.com/index.php?topic=6013.0

	NFITC1s Proud Clod
		http://forums.qhimm.com/index.php?topic=8481.0
	
	Haruhiko Okumuras LZSS.C
		http://www.das-labor.org/svn/fpga/farbborg/firmware/bootloader-sd/compressor-pc.c

	Icon fixed by DLPB
	Uses Document Icon by kyo-tux:
		http://kyo-tux.deviantart.com/gallery/?offset=24#/d2dcv72
	And the Team Avalanche GUI accessory icon
		http://forums.qhimm.com/index.php?topic=9151.0
------------------------------------------------------------------
