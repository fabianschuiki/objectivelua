
-- Create a table for debugging the table functionality.
t = {
    id = condo,
    name = "Heron's Loft",
    price = 45000,
    categories = {
        a = 9,
        b = "buh"
    },
	isValid = true,
    test = {
    	"a", 5, "alpha"
    }
}

-- Create a new sprite for debugging purposes.
local s1 = Sprite:new()
s1.name = "Office"
local s2 = SpecialSprite:new()
s2.name = "Fast Food"

dump(s1)
dump(s2)

s1:say()
s2:say()

s2:speciality()
s1:animate()
s2:animate()

return s1

--return "Hello", t
