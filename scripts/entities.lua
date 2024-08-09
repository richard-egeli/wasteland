Entity_Defs = {}

Player = nil

Collider = {
	STATIC = 0,
	DYNAMIC = 1,
}

Entity_Defs["player"] = {
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

Entity_Defs["tree_01"] = {
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

Entity_Defs["house_01"] = {
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

Entity_Defs["house_02"] = {
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

function on_entity_spawn(x, y, name)
	local def = Entity_Defs[string.lower(name)]

	if def ~= nil then
		local entity = create_entity(x, y, def)
		if name == "Player" then
			Player = entity
		end
	end
end
