require("scripts/input")

local world = World.new()

---@class Player : Entity
---@field health number
local ent = world:create_entity({
	health = 90,
	sprite = PLAYER_SPRITE,

	---@param self Player
	update = function(self)
		local speed = 5.0
		local x, y = self:get_position()

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

		self:set_position(x, y)
	end,
})

ent:set_position(50, 50)
print(ent:get_position())

-- local child = world:create_entity({
-- 	parent = ent,
-- 	update = function()
-- 		print("Child")
-- 	end,
-- })
