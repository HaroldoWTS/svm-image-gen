function sample_training_points(n)
	local ret = {}
	local x = 0.0
	local y = 0.0
	local label
	for i = 1,n do
		x = math.random()*10 - 5
		y = math.random()*10 - 5
		label = (x+y > 0.0)
		ret[i] = {point={x,y}, label=label}
	end
	ret[n+1] = {point={-1.0, -1.0}, label=false}
	ret[n+2] = {point={1.0, 1.0}, label=true}
	return ret
end

training_set = sample_training_points(1000)

for i,p in next,training_set do
--	print(p.point[1]," ",p.point[2],p.label)
end

test_set = {
	{point={2,2}, label=true},
	{point={-2,3}, label=true},
	{point={1,-3}, label=false},
	{point={-2,-2}, label=false}
}

kernel = function(u,v)
	ret = 0
	for i=1,#u do
		ret = ret + u[i]*v[i]
	end
	return ret
end

C = 10000

print(#training_set, " pontos!")

trained = svm.train(training_set, kernel, C)

for t=1,#test_set do
	print(trained(test_set[t].point, kernel) == test_set[t].label)
end

