math.randomseed(2345678)
local test = require "test"
local kernels = require "kernel"

function sample_training_points_plano(n)
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

function sample_training_points_quadcircle(n)
--O ponto deve estar dentro do quadrado (4,1) (5,8)
--mas fora do circulo de centro (6,4) e raio 1
	local ret = {}
	local x = 0.0
	local y = 0.0
	local rx
	local ry
	local point
	for i = 1,n do
		x = math.random()*20 - 10
		y = math.random()*20 - 10
		rx = x - 6
		ry = y - 3
		point = {point={x/10.0,y/10.0}}
		if x > 4.0 and x < 8.0 and y > 1.0 and y < 5 and (rx*rx + ry*ry) > 1.0 then
			point.label = true	
		else
			point.label = false
		end
		ret[i] = point
	end
	return ret
end

test_set = {
	{point={2,2}, label=true},
	{point={-2,3}, label=true},
	{point={1,-3}, label=false},
	{point={-2,-2}, label=false}
}


local C = 10000

svm_train = function(tset)
	return svm.train(tset, kernels.get_poly(0.5, 4), C )
end

test.svm(svm_train, sample_training_points_quadcircle, 1000)



