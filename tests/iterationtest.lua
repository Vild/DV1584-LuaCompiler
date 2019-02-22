iterationCounter = 0

function func(a, b)
	iterationCounter = iterationCounter + 1
	if b == 0 then
		 return 0
	end
	local x = a - b
	local y = func(a, b - 1)
	return x + y
end

print(func(13, 37))
