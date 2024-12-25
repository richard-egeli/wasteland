print("NOW")

print("ID Now")
print(PLAYER_SPRITE)
print(CHAR_SPRITE)

local world = World.new()

local ent = world:create_entity({
	update = function()
		print("Parent")
	end,
})

-- local child = world:create_entity({
-- 	parent = ent,
-- 	update = function()
-- 		print("Child")
-- 	end,
-- })
