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


	c = svm.chinksolve(D,y,C)

	print(#c,"vetores de suporte")
	for i,ci in next,c do
		print("c",i,ci)
	end

	--um vetor de suporte deve ser escolhido e sera o primeiro
	supi = next(c)

	--calculando b (direto da wikipedia)
	b = 0.0 - y[supi]
	for j,cj in next,c do
		b = b + cj*y[j]*K[supi][j]
	end

	print("b:",b)

	return function(z)
		local rsum = 0.0
		for i,ci in next,c do
			rsum = rsum + ci*y[i]*kernel(pontos[i].point, z )
		end
		return (rsum - b) > 0
	end
end

--algoritmo SMO + WSS3
--https://www.csie.ntu.edu.tw/~cjlin/papers/quadworkset.pdf
svm.chinksolve = function(Q, y, C )

len = #y
local eps = 1e-3
local tau = 1e-12
local A = {}
local G = {}
local i
local j
local a
local oldAi
local oldAj
local sum
local deltaAi
local deltaAj
local b

for it=1,len do
	A[it] = 0.0
	G[it] = -1.0
end

while true do
	i,j = svm.selectB(Q, A, G, y, C, tau, eps)
	if (j == -1) then
		break
	end
	a = Q[i][i] + Q[j][j] - 2*y[i]*y[j]*Q[i][j]
	if (a <= 0) then
		a = tau
	end
	b = -1.0*y[i]*G[i] + y[j]*G[j]

	--update alpha
	oldAi = A[i]
	oldAj = A[j]
	A[i] = A[i] + y[i]*b/a
	A[j] = A[j] - y[j]*b/a

	--project alpha back to the feasible region
	sum = y[i]*oldAi + y[j]*oldAj
	if A[i] > C then
		A[i] = C
	elseif A[i] < 0.0 then
		A[i] = 0.0
	end
	A[j] = y[j]*(sum -y[i]*A[i])
	if A[j] > C then
		A[j] = C
	elseif A[j] < 0.0 then
		A[j] = 0.0
	end
	A[i] = y[i]*(sum -y[j]*A[j])

	--update gradient
	deltaAi = A[i] - oldAi
	deltaAj = A[j] - oldAj
	for t = 1,len do
		G[t] = G[t] + Q[t][i]*deltaAi + Q[t][j]*deltaAj
	end
end
	local ret = {}
	for t=1,len do
		if (A[t]*A[t] > tau) then
			ret[t] = A[t]
		end
	end
	return ret

end

svm.selectB = function(Q, A, G, y, C, tau, eps) 
	local i = -1
	local j
	local len = #y
	local G_max = "inf"
	local G_min = "-inf"
	local b
	
	for t = 1,len do
		if (y[t] == 1 and A[t] < C) or
			(y[t] == -1 and A[t] > 0) then
			if (G_max == "inf" or -y[t]*G[t] >= G_max) then
				i = t
				G_max = -1*y[t]*G[t]
			end
		end
	end

	j = -1
	obj_min = "inf"
	for t=1,len do
		if (y[t] == 1 and A[t] > 0) or
			(y[t] == -1 and A[t] < C) then
			b = G_max + y[t]*G[t]
			if (G_min == "-inf" or -1*y[t]*G[t] <= G_min) then
				G_min = -1*y[t]*G[t]
			end
			if (b >0) then
				a=Q[i][i]+Q[t][t]-2*y[i]*y[t]*Q[i][t]
				if (a <= 0) then
					a = tau
				end
				if (obj_min == "inf" or -1*(b*b)/a <= obj_min) then
					j = t
					obj_min = -1*(b*b)*a
				end
			end
		end
	end
	if (G_max-G_min < eps) then
		return -1,-1
	else
		return i,j
	end
end
