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

function NPC:update(delta)
	local posx, posy = self.entity:get_position()
	local target = self.path[self.path_index]
	local tarx = target.x * 16
	local tary = target.y * 16
	local speed = 50

	local dirx = tarx - posx
	local diry = tary - posy
	local dist = math.sqrt(dirx ^ 2 + diry ^ 2)

	if dist > 0 then
		dirx = dirx / dist
		diry = diry / dist

		local move_step = speed * delta

		if dist <= move_step then
			self.entity:set_position(tarx, tary)
			self.path_index = self.path_index + 1

			if self.path_index > #self.path then
				self.path_index = 1
			end
		else
			self.entity:move(move_step * dirx, move_step * diry)
		end
	end
end
