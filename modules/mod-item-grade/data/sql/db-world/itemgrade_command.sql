-- mod-item-grade GM 명령어 도움말/보안 등록 (command 테이블)
DELETE FROM `command` WHERE `name` IN ('itemgrade', 'itemgrade add', 'itemgrade info');
INSERT INTO `command` (`name`, `security`, `help`) VALUES
('itemgrade', 2, 'Syntax: .itemgrade <add|info>\nItem grade(S/A/B/C/D) GM commands.'),
('itemgrade add', 2, 'Syntax: .itemgrade add <itemId|itemlink> <S/A/B/C/D>\nCreates an item with the given grade and gives it to yourself. Grade also accepts 0~4 (0=D .. 4=S).'),
('itemgrade info', 2, 'Syntax: .itemgrade info\nShows grade and per-stat base->scaled values of your equipped items (verification).');
