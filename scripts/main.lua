require("scripts.entities")

local player_speed = 100.0

---@class Projectile
---@field entity Entity
---@field direction table
---@field velocity number
---@field created number
Projectile = {}
Projectile.__index = Projectile

function Projectile:new(entity, direction, velocity)
	local instance = setmetatable({}, Projectile)
	instance.created = get_time()
	---@class Entity
	instance.entity = entity
	instance.direction = direction
	instance.velocity = velocity
	return instance
end

---@type Projectile[]
local projectiles = {}

---@type Entity
Player = nil

function update(delta_time)
	if Player ~= nil then
		if action_down("move_up") then
			Player:move(0, -1.0 * player_speed * delta_time)
		end

		if action_down("move_down") then
			Player:move(0, 1.0 * player_speed * delta_time)
		end

		if action_down("move_left") then
			Player:move(-1.0 * player_speed * delta_time, 0)
		end

		if action_down("move_right") then
			Player:move(1.0 * player_speed * delta_time, 0)
		end

		if action_down("space") then
			local x, y = Player:get_position()
			local dx, dy = Player:get_mouse_direction()
			local entity = create_entity(x, y, Entity_Defs["bullet"])
			local proj = Projectile:new(entity, { x = -dx, y = -dy }, 300)
			table.insert(projectiles, proj)
		end
	end

	for index, value in ipairs(projectiles) do
		local x = value.velocity * value.direction.x * delta_time
		local y = value.velocity * value.direction.y * delta_time
		value.entity:move(x, y)

		local time = get_time() - value.created

		if time > 1.0 then
			value.entity:destroy()
			table.remove(projectiles, index)
		end
	end
end

---@param e1 Entity
---@param e2 Entity
function collision(e1, e2)
	print(Entities[e1].name .. " colliding with " .. Entities[e2].name)
end
