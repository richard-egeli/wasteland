-- Check if an action is down
---@param key string the name of the action
---@return boolean is_down if the key is down this frame
function action_down(key)
	error("function implemented in C and should not be called in Lua")
end

-- Check if an action is up
---@param key string the name of the action
---@return boolean is_up the key is up this frame
function action_up(key)
	error("function implemented in C and should not be called in Lua")
end

-- Is the action pressed this frame
-- @param key is the name of the action
-- @return bool true if pressed this frame
function action_pressed(key)
	error("function implemented in C and should not be called in Lua")
end

--- Is the action released this frame
---@param key string the name of the action
---@return boolean released if the action was released this frame
function action_released(key)
	error("function implemented in C and should not be called in Lua")
end

--- Create a new entity at position
---@param x number the X position of the entity
---@param y number the Y position of the entity
---@param def table the definition of the entity
---@return Entity the entity created
function create_entity(x, y, def)
	error("function implemented in C and should not be called in Lua")
end

--- Get mouse position in world space
---@return number x, number y
function get_mouse_world_position()
	error("function implemented in C and should not be called in Lua")
end

--- Get the current time
---@return number time
function get_time()
	error("function implemented in C and should not be called in Lua")
end

---@class Entity
---@field name string
Entity = {}

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
