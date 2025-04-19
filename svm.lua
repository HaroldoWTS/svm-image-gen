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
	
	K = function(i, j)
		return kernel(pontos[i].point, pontos[j].point)
	end

	--c = svm.chinksolve(D,y,C)
	print("Chamando C")
	c = svm.solve_smo_wss3(K,y,C)

	print(#c,"vetores de suporte")
	for i,ci in next,c do
		print("c",i,ci)
	end

	--um vetor de suporte deve ser escolhido e sera o primeiro
	supi = next(c)

	--calculando b (direto da wikipedia)
	--D(i,j) = K(i,j)*yi*yj
	b = 0.0 - y[supi]
	for j,cj in next,c do
		b = b + cj*y[j]*K(supi, j)
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
local yi
local yj

for it=1,len do
	A[it] = 0.0
	G[it] = -1.0
end

while true do
	i,j = svm.selectB(Q, A, G, y, C, tau, eps)
	yi = y[i]
	yj = y[j]
	if (j == -1) then
		break
	end
	a = Q[i][i] + Q[j][j] - 2*yi*yj*Q[i][j]
	if (a <= 0) then
		a = tau
	end
	b = -1.0*yi*G[i] + yj*G[j]

	--update alpha
	oldAi = A[i]
	oldAj = A[j]
	A[i] = A[i] + yi*b/a
	A[j] = A[j] - yj*b/a

	--project alpha back to the feasible region
	sum = yi*oldAi + yj*oldAj
	if A[i] > C then
		A[i] = C
	elseif A[i] < 0.0 then
		A[i] = 0.0
	end
	A[j] = yj*(sum -yi*A[i])
	if A[j] > C then
		A[j] = C
	elseif A[j] < 0.0 then
		A[j] = 0.0
	end
	A[i] = yi*(sum -yj*A[j])

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
	local yt
	local Qi

	for t = 1,len do
		yt = y[t]
		if (yt == 1 and A[t] < C) or
			(yt == -1 and A[t] > 0) then
			if (G_max == "inf" or -yt*G[t] >= G_max) then
				i = t
				G_max = -1*yt*G[t]
			end
		end
	end

	Qi = Q[i]

	j = -1
	obj_min = "inf"
	for t=1,len do
		yt = y[t]
		if (yt == 1 and A[t] > 0) or
			(yt == -1 and A[t] < C) then
			b = G_max + yt*G[t]
			if (G_min == "-inf" or -1*yt*G[t] <= G_min) then
				G_min = -1*yt*G[t]
			end
			if (b >0) then
				a=Qi[i]+Q[t][t]-2*y[i]*yt*Qi[t]
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
