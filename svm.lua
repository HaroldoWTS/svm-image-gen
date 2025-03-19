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

	if n < 2 then
		print(n," pontos é muito pouco!")
		return nil
	end

	--i é linha, j é coluna
	for i = 1,n do
		y[i] = (pontos[i].label and 1.0) or -1.0
	end

	--calculando K diagonal superior
	for j = 1,n do 
		xj = pontos[j].point
		Kj = {}
		for i=1,j do
			xi = pontos[i].point
			Kj[i] =kernel(xi, xj)
		end
		K[j] = Kj
	end

	--K é simétrica
	for j = 1,n do 
		for i=j+1,n do
			K[j][i] = K[i][j]
		end
	end

	for j =1,n do
		yj = y[j]
		Kj = K[j]
		Dj = {}
		for i=1,n do
			Dj[i] = y[i]*yj*Kj[i]
		end
		D[j] = Dj
	end

	c = svm.solve(D,y,C)

	--um vetor de suporte deve ser escolhido e sera o primeiro
	supi = next(c)

	--calculando b (direto da wikipedia)
	b = 0.0 - y[supi]
	for j,cj in next,c do
		b = b + cj*y[j]*K[supi][j]
	end

	return function(z)
		local rsum = 0.0
		for i,ci in next,c do
			rsum = rsum + ci*y[i]*kernel(pontos[i].point, z )
		end
		return (rsum - b) > 0
	end
end

