--[[
This file is supposed to extend a basic C class which provides some functions
and callbacks.
--]]

-- Implement the animation function. This is a function that is part of the C++
-- class and is exposed to Lua and thus extensible.
function Sprite:animate()
	print("Sprite:animate(), sprite name = " .. self.name)
end
