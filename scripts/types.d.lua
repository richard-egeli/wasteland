---@diagnostic disable: missing-fields

---@meta

---@class Entity
---@field load? fun(self: Entity)
---@field update? fun(self: Entity)
---@field render? fun(self:Entity)
---@field parent? Entity

---@class World
---@field new fun(): World
---@field create_entity fun(self:World, entity: Entity): Entity

---@class AnimationFrame
---@field column integer
---@field row integer
---@field duration number
---@field invert boolean

---@class Animation
---@field name string
---@field frames AnimationFrame[]
---@field loop boolean

---@class AssetLoader
---@field load_spritesheet fun(path:string, cols:number, rows:number, animations: Animation[]): number

---@type World
World = {}

---@type AssetLoader
AssetLoader = {}
