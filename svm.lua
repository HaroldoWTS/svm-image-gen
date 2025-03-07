--[[training_set = {
	{{-1, -1}, false},
	{{1,1}, true}
}

kernel = svm.dot

C = 100

trained = svm.train(training_set, kernel, C)
print(trained({2,2}, kernel))
--]]

L = {{2,0}, {2,2}}
A = {{-1, 1, 0}, {1,0,1}}
C = 100
q = {-1, -1}
l = {0, 0, 0}
u = {0, C, C}

print('a')
Lcsc = osqp.matriz(L)
print('aa')
osqp.printar_csc(Lcsc)
print('b')
Acsc = osqp.matriz(A)
osqp.printar_csc(Acsc)
print('c')
qv = osqp.vetor(q)
osqp.printar_vetor(qv, 2)
print('d')
lv = osqp.vetor(l)
osqp.printar_vetor(lv, 3)
print('e')
uv = osqp.vetor(u)
osqp.printar_vetor(uv, 3)


rv = osqp.solucionar(Lcsc,qv, Acsc, lv, uv)
osqp.printar_vetor(rv, 2)



