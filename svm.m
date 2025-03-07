P = [[-1 -1]' [1 1]' ]
 
y = [-1 1 ]'

n = length(y)
D = eye(n,n)


for i = 1:n
	for j = 1:n
		D(i,j) = y(i)*y(j)*(P(:,i)'*P(:,j) )
	endfor
endfor

q = -1*ones(n,1)
b = 0
A_lb = [0 0 0]'
A_ub = [0 100 100]'
A_in = [y'; eye(2,2)]
H = D

[L, obj, info] = qp([24 24]', H, q, [], [], [], [], A_lb, A_in, A_ub )



