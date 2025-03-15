training_set = {
	{point={-1, -1}, label=false},
	{point={1,1}, label=true}
}

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

C = 100

trained = svm.train(training_set, kernel, C)

for t=1,#test_set do
	print(trained(test_set[t].point, kernel) == test_set[t].label)
end

