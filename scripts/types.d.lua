---@diagnostic disable: missing-fields

---@meta

---@class Entity
---@field get_position fun(self:Entity): x:number, y:number
---@field set_position fun(self:Entity, x:number,y:number)

---@class EntityDef
---@field sprite? number
---@field load? fun(self: Entity)
---@field update? fun(self: Entity)
---@field parent? Entity

---@class DynamicBodyDef
---@field x number
---@field y number
---@field parent? any

---@class World
---@field new fun(): World
---@field create_entity fun(self:World, entity: EntityDef): any
---@field create_dynamic_body fun(self:World, definition: DynamicBodyDef): any

---@class Input
---@field is_down fun(key:integer): boolean
---@field is_up fun(key:integer):boolean
---@field is_pressed fun(key:integer):boolean
---@field is_released fun(key:integer):boolean

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

---@type Input
Input = {}
