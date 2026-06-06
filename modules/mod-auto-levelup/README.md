# mod-auto-levelup

Per account, the first currently existing non-death-knight character
starts at a custom level. Every later non-death-knight character starts
at another configured level.

Death knights are ignored by this module. They do not receive the custom
start level and do not count when deciding whether a later character is
the first eligible character.

Default behavior:
- first current character: level 70
- later characters: level 1
- death knights: unchanged

Behavior example:
- account creates first character -> level 70
- account creates second character -> level 1
- account creates death knight first -> unchanged
- account creates first non-death-knight after that -> level 70
- account deletes all characters
- account creates a new character again -> level 70

Config:
- `AutoLevelup.Enable`
- `AutoLevelup.FirstCharacterLevel`
- `AutoLevelup.OtherCharacterLevel`
