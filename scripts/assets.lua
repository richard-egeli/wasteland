---@type Animation[]
local human_animations = {
	{
		name = "idle_down",
		loop = false,
		frames = {
			{
				row = 1,
				column = 0,
				duration = 0,
				invert = true,
			},
		},
	},
	{
		name = "idle_left",
		loop = false,
		frames = {
			{
				column = 1,
				row = 1,
				duration = 0,
				invert = false,
			},
		},
	},
	{
		name = "idle_right",
		loop = false,
		frames = {
			{
				column = 1,
				row = 2,
				duration = 0,
				invert = false,
			},
		},
	},
	{
		name = "idle_up",
		loop = false,
		frames = {
			{
				column = 1,
				row = 3,
				duration = 0,
				invert = false,
			},
		},
	},
	{
		name = "walk_down",
		loop = true,
		frames = {},
	},
	{
		name = "walk_right",
		loop = true,
		frames = {},
	},
	{
		name = "walk_left",
		loop = true,
		frames = {},
	},
	{
		name = "walk_up",
		loop = true,
		frames = {},
	},
}

PLAYER_SPRITE = AssetLoader.load_spritesheet("assets/sprite/character_1-8.png", 12, 8, human_animations)
CHAR_SPRITE = AssetLoader.load_spritesheet("assets/sprite/character_17-24.png", 12, 8, human_animations)
