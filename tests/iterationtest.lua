iterationCounter = 0

function fun(a, b)
	iterationCounter = iterationCounter + 1
	if b == 0 then
		 return 0
	end
	local x = a - b
	local y = func(a, b - 1)
	return x + y
end

func(13, 37)