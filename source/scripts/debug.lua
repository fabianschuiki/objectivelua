print("Hello, World!")

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

function test(tab)
	print("test caled!")
	dumpStack()
end

test(t);

return "Hello", t
