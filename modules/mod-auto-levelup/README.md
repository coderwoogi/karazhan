# mod-auto-levelup

Per account, the first currently existing character starts at a custom
level. Every later character starts at another configured level.

Default behavior:
- first current character: level 70
- later characters: level 1

Behavior example:
- account creates first character -> level 70
- account creates second character -> level 1
- account deletes all characters
- account creates a new character again -> level 70

Config:
- `AutoLevelup.Enable`
- `AutoLevelup.FirstCharacterLevel`
- `AutoLevelup.OtherCharacterLevel`
