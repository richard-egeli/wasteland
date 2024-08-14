---@class Path
---@field x number
---@field y number
Path = {}

---@class NPC
---@field path_index integer
---@field entity Entity
---@field path Path[] array of points
NPC = {}
NPC.__index = NPC

function NPC:new(entity)
	local instance = setmetatable({}, NPC)
	instance.path = entity["Path"]
	instance.path_index = 1
	instance.entity = entity
	return instance
end

function NPC:follow_grid(delta)
	local path = self.path[self.path_index]
	local tarx = path.x * 16
	local tary = path.y * 16
	local posx, posy = self.entity:get_position()
	local speed = 50

	local dirx = tarx - posx
	local diry = tary - posy
	local dist = math.sqrt(dirx ^ 2 + diry ^ 2)
	local move_step = speed * delta

	if dist > 0 then
		dirx = dirx / dist
		diry = diry / dist
		if dist <= 2 then
			self.entity:set_position(tarx, tary)
		else
			self.entity:move(move_step * dirx, move_step * diry)
		end
	else
		self.path_index = self.path_index + 1
		if self.path_index > #self.path then
			self.path_index = 1
		end
	end
end

function NPC:update(delta, target)
	local posx, posy = self.entity:get_position()
	local tarx, tary = target:get_position()
	local speed = 50

	local dirx = tarx - posx
	local diry = tary - posy
	local dist = math.sqrt(dirx ^ 2 + diry ^ 2)

	if dist > 0 then
		dirx = dirx / dist
		diry = diry / dist

		local move_step = speed * delta
		self.entity:move(move_step * dirx, move_step * diry)
	end
end
