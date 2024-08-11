---@class Entity
---@field name string
---@field speed number
Entity = {}
Entity.__index = Entity

--- Set the position directly
---@param x number the x coordinate
---@param y number the y coordinate
function Entity:set_position(x, y)
	error("function implemented in C and should not be called in Lua")
end

---@return number x, number y position of the entity in coordinates
function Entity:get_position()
	error("function implemented in C and should not be called in Lua")
end

--- Get the direction of the mouse compared to the entity position
---@return number x, number y a normalized direction vector relative to position
function Entity:get_mouse_direction()
	error("function implemented in C and should not be called in Lua")
end

--- Move the entity by x,y
---@param x number
---@param y number
function Entity:move(x, y)
	error("function implemented in C and should not be called in Lua")
end

--- Mark the entity for destruction
function Entity:destroy()
	error("function implemented in C and should not be called in Lua")
end

--- Retrieve the ID used internally in C
---@return integer id
function Entity:get_id()
	error("function implemented in C and should not be called in Lua")
end
