---@diagnostic disable: missing-fields

---@meta

---@class Entity
---@field get_position fun(self:Entity): x:number, y:number
---@field set_position fun(self:Entity, x:number,y:number)

---@class Body : Entity
---@field get_type fun(self:Body): integer

---@class Dynamic : Body
---@field move fun(self: Dynamic, x: number, y:number)
---@field children table

---@class Static : Body
---@field children table

---@class Sprite : Body
---@field children table
---@field set_cell fun(self:Sprite, col: integer, row:integer)

---@class SpriteDef
---@field sprite integer
---@field row? integer
---@field col? integer

---@class EntityDef
---@field sprite? number
---@field load? fun(self: Entity)
---@field update? fun(self: Entity)
---@field parent? Entity

---@class DynamicDef
---@field x? number
---@field y? number
---@field parent? Entity

---@class StaticDef
---@field x? number
---@field y? number
---@field parent? Entity

---@class BoxColliderDef
---@field x? number
---@field y? number
---@field width number
---@field height number
---@field parent? Entity
---@field trigger? boolean
---@field on_collision_enter? fun(self:Static, other:any)
---@field on_collision_exit? fun(self:Static, other:any)

---@class World
---@field children table
---@field create_entity fun(self:World, entity: EntityDef): any
---@field create_dynamic_body fun(self:World, definition: DynamicDef): any
---@field create_static_body fun(self:World, definition:StaticDef): any
---@field create_sprite fun(self:World, definition:SpriteDef): any
---@field create_box_collider fun(self:World, definition: BoxColliderDef): any

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

---@class WorldAPI
---@field new fun(): World
World = {}

---@type AssetLoader
AssetLoader = {}

---@type Input
Input = {}
