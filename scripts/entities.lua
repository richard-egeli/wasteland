Entity_Defs = {}

Collider = {
	STATIC = 0,
	DYNAMIC = 1,
}

LayerMask = {
	NONE = 0,
	GROUND = 1,
	PLAYER = 2,
	NPC = 4,
	PROJECTILE = 8,
}

Entity_Defs["Bullet"] = {
	sprite = {
		texture = "assets/player.png",
		cell_x = 0,
		cell_y = 0,
		cell_width = 14,
		cell_height = 14,
		grid_size = 14,
		sort_point = 14,
	},

	box_collider = {
		type = Collider.DYNAMIC,
		mask = LayerMask.PROJECTILE,
		trigger = true,
		origin = {
			x = 0,
			y = 0,
		},
		position = {
			x = 0,
			y = 0,
		},
		size = {
			x = 14,
			y = 14,
		},
	},
}

Entity_Defs["NPC"] = {
	sprite = {
		texture = "assets/player.png",
		cell_x = 0,
		cell_y = 0,
		cell_width = 16,
		cell_height = 16,
		grid_size = 16,
		sort_point = 16,
	},

	box_collider = {
		type = Collider.DYNAMIC,
		mask = LayerMask.GROUND + LayerMask.PROJECTILE + LayerMask.NPC,
		origin = {
			x = 0,
			y = 0,
		},
		position = {
			x = 0,
			y = 0,
		},
		size = {
			x = 16,
			y = 16,
		},
	},
}

Entity_Defs["Player"] = {
	sprite = {
		texture = "assets/player.png",
		cell_x = 0,
		cell_y = 0,
		cell_width = 16,
		cell_height = 16,
		grid_size = 16,
		sort_point = 16,
	},

	box_collider = {
		type = Collider.DYNAMIC,
		mask = LayerMask.GROUND + LayerMask.PLAYER,
		trigger = false,
		origin = {
			x = 0,
			y = 0,
		},
		position = {
			x = 0,
			y = 0,
		},
		size = {
			x = 16,
			y = 16,
		},
	},
}

Entity_Defs["Tree_01"] = {
	sprite = {
		texture = "assets/atlas.png",
		cell_x = 464,
		cell_y = 96,
		cell_width = 32,
		cell_height = 48,
		grid_size = 16,
		sort_point = 44,
	},

	box_collider = {
		type = Collider.STATIC,
		mask = LayerMask.GROUND,
		debug = true,
		origin = {
			x = 10,
			y = 34,
		},

		position = {
			x = 0,
			y = 0,
		},

		size = {
			x = 12,
			y = 12,
		},
	},
}

Entity_Defs["House_01"] = {
	sprite = {
		texture = "assets/atlas.png",
		cell_x = 128,
		cell_y = 256,
		cell_width = 64,
		cell_height = 48,
		grid_size = 16,
		sort_point = 32,
	},
}

Entity_Defs["House_02"] = {
	sprite = {
		texture = "assets/atlas.png",
		cell_x = 192,
		cell_y = 256,
		cell_width = 64,
		cell_height = 48,
		grid_size = 16,
		sort_point = 32,
	},
}

Entity_Defs["Door"] = {
	box_collider = {
		type = Collider.STATIC,
		mask = LayerMask.GROUND,
		trigger = true,
		debug = true,

		position = {
			x = 0,
			y = 0,
		},

		size = {
			x = 16,
			y = 16,
		},
	},
}
