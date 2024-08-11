require("scripts.entities")
require("scripts.npc")

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

---@type NPC[]
local npcs = {}

---@type Entity
Player = nil

---@type NPC
Test = nil

Level = load_level("assets/test.ldtk", "Level_0")

Level:load_entities("Entities", function(x, y, name, ...)
	local args = { ... }
	local definition = Entity_Defs[name]

	if definition ~= nil then
		local entity = Level:spawn_entity(x, y, definition)
		local entity_id = entity:get_id()
		entity.name = name

		Entities[entity_id] = entity
		if name == "Player" then
			Player = entity
			Player.speed = 100.0
		end

		for i = 1, #args, 2 do
			local id = args[i]
			local value = args[i + 1]
			if id ~= nil and value ~= nil then
				print(id)
				entity[id] = value
			end
		end

		if name == "NPC" then
			table.insert(npcs, NPC:new(entity))
		end
	end
end)

function update(delta_time)
	local player_speed = Player.speed
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
			local entity = Level:spawn_entity(x, y, Entity_Defs["Bullet"])
			local proj = Projectile:new(entity, { x = -dx, y = -dy }, 300)
			table.insert(projectiles, proj)
		end

		for _, npc in ipairs(npcs) do
			npc:update(delta_time)
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
	local ent1 = Entities[e1]
	local ent2 = Entities[e2]

	if ent1["Test"] == "Teleport" then
		local p = ent1["Point"]
		ent2:set_position(p.x * 16, p.y * 16)
	end
end
