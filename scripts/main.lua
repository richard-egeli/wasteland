require("scripts/input")

local world = World.new()

---@class Player : Entity
---@field health number
local ent = world:create_dynamic_body({
	health = 90,
	sprite = PLAYER_SPRITE,

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

world:create_static_body({
	x = 64,
	y = 0,
})
