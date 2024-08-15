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
	instance.entity = entity
	instance.created = get_time()
	instance.direction = direction
	instance.velocity = velocity
	return instance
end

---@type Projectile[]
local projectiles = {}

---@type NPC[]
local npcs = {}

local collision_events = {}

---@type Entity[]
local weak_entity_table = setmetatable({}, { __mode = "v" })

---@type Entity[]
local world_entity_table = {}

---@type Entity
Player = nil

---@type NPC
Test = nil

Level = load_level("assets/2.5D.ldtk", "Level_0")

action_register("move_right", string.byte("D"))
action_register("move_left", string.byte("A"))
action_register("move_down", string.byte("S"))
action_register("move_up", string.byte("W"))
action_register("space", string.byte(" "))

Level:load_entities("Entities", function(x, y, name, ...)
	local args = { ... }
	local definition = Entity_Defs[name]

	if definition ~= nil then
		local entity = Level:spawn_entity(x, y, definition)
		entity.name = name

		for i = 1, #args, 2 do
			local id = args[i]
			local value = args[i + 1]
			if id ~= nil and value ~= nil then
				entity[id] = value
			end
		end

		if name == "Player" then
			Player = entity
			Player.speed = 100.0
			return
		end

		if name == "NPC" then
			table.insert(npcs, NPC:new(entity))
			weak_entity_table[entity:get_id()] = entity
			return
		end

		world_entity_table[entity:get_id()] = entity
	end
end)

local spawn_rate = 3.0
local last_spawn = get_time()

local function spawn_enemy()
	if get_time() - last_spawn > spawn_rate then
		local x = 640 * math.random()
		local y = 360 * math.random()

		local entity = Level:spawn_entity(x, y, Entity_Defs["NPC"])
		entity.name = "NPC"
		weak_entity_table[entity:get_id()] = entity
		table.insert(npcs, NPC:new(entity))
		last_spawn = get_time()
		spawn_rate = spawn_rate - 0.1

		if spawn_rate < 0.001 then
			spawn_rate = 0.001
		end
	end
end

local function solve_events()
	for _, event in ipairs(collision_events) do
		---@type NPC
		local npc = event.npc
		local index = event.index

		table.remove(npcs, index)
		npc.entity:destroy()
	end

	collision_events = {}
end

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
			weak_entity_table[entity:get_id()] = entity
			entity.name = "Bullet"
			table.insert(projectiles, proj)
		end

		for _, npc in ipairs(npcs) do
			-- npc:follow_grid(delta_time)
			npc:update(delta_time, Player)
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

	spawn_enemy()
	solve_events()
end

---@param e1 userdata
---@param e2 userdata
function collision(e1, e2)
	local ent1 = weak_entity_table[e1]
	local ent2 = weak_entity_table[e2]
	if ent1 == nil or ent2 == nil or ent1.name == nil or ent2.name == nil then
		return
	end

	if ent1.name == "Player" and ent2.name == "NPC" then
		for index, value in ipairs(npcs) do
			if value.entity:get_id() == e2 then
				value.entity:destroy()
				table.remove(npcs, index)
				break
			end
		end
	end

	if ent1.name == "Bullet" and ent2.name == "NPC" then
		for index, value in ipairs(npcs) do
			if value.entity:get_id() == e2 then
				value.entity:destroy()
				table.remove(npcs, index)
				break
			end
		end
	end
end
