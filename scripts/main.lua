require("scripts/input")

local world = World.new()

---@class Player : Dynamic
---@field health number
local ent = world:create_dynamic_body({
	health = 90,

	---@param self Player
	update = function(self)
		local speed = 50.0
		local x, y = 0, 0

		if Input.is_down(KeyboardKey.W) then
			y = y - speed
		end

		if Input.is_down(KeyboardKey.S) then
			y = y + speed
		end

		if Input.is_down(KeyboardKey.A) then
			x = x - speed
		end

		if Input.is_down(KeyboardKey.D) then
			x = x + speed
		end

		self:move(x, y)
	end,
})

ent.children = {
	sprite = world:create_sprite({
		parent = ent,
		sprite = PLAYER_SPRITE,
		row = 0,
		col = 0,
	}),
}

local static = world:create_static_body({
	name = "St John",
	x = 64,
	y = 0,

	on_collision_enter = function(self, other)
		print("Collision Entered!", self.name, other.health)
	end,

	on_collision_exit = function(self, other)
		print("Collision Exited!", other.health)
	end,
})
