local kernel = {}

kernel.dot = function(u,v)
	local ret = 0
	for i=1,#u do
		ret = ret + u[i]*v[i]
	end
	return ret
end

kernel.get_dot = function()
	return kernel.dot
end

kernel.get_poly = function(c,d)
	return function(u,v)
		local dc = kernel.dot(u,v) + c
		local ret = 1
		local i
		for i=1,d do
			ret = ret*dc
		end
		return ret
	end
end

return kernel
