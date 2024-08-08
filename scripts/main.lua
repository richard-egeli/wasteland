require("scripts.entities")

local player_speed = 100.0

function update(delta_time)
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
end
