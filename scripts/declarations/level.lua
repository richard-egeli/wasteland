---@class Level
Level = {}

--- Loads entities from a layer of a LDTK level
---@param key string the name of the layer
---@param callback function the callback function with entity information
function Level:load_entities(key, callback)
	error("function implemented in C and should not be called in Lua")
end

--- Spawns an entity in the level at position x,y with the definition
--- @param x number the x coordinate in the level
--- @param y number the y coordinate in the level
--- @param def table the table definition of the entity
--- @return Entity entity entity class
function Level:spawn_entity(x, y, def)
	error("function implemented in C and should not be called in Lua")
end
