svm.train = function(pontos, kernel, C)
	-- column major "matrix" of K(x_i, x_j) 
	local K = {}
	local y = {} --vetor de rotulos
	local n = #pontos
	local m = n+1
	local xj
	local xi
	local Kj
	local Aj 
	local c -- o vetor solucao
	local supi --indice de um vetor de suporte
	local b --bias do plano
	local D = {}

	--i é linha, j é coluna
	for i = 1,n do
		y[i] = (pontos[i].label and 1.0) or -1.0
	end

	--TODO: n calcular duas vezes o kernel, matriz simetrica
	for j = 1,n do
		xj = pontos[j].point
		Kj = {}
		for i=1,n do
			xi = pontos[i].point
			Kj[i] =kernel(xi, xj)
		end
		K[j] = Kj
	end

	for j =1,n do
		Dj = {}
		for i=1,n do
			Dj[i] = y[i]*y[j]*K[j][i]
		end
		D[j] = Dj
	end

	c = svm.solve(D,y,C)
	for i = 1,n do
		if c[i] ~= 0.0 then
			supi = i
			break
		end
	end

	local sum = 0.0
	for j = 1,n do
		sum = sum + c[j]*y[j]*K[supi][j]
	end
	b = sum - y[supi]

	return function(z)
		local rsum = 0.0
		for i =1,n do
			rsum = rsum + ((c[i] == 0.0 and 0.0 ) or c[i]*y[i]*kernel(pontos[i].point, z ))
		end
		return (rsum - b) > 0
	end

end





