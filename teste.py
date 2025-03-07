import numpy as np
import scipy as sp
import osqp

P = np.array([[-1,-1], [2,2]]).T
y = np.array([-1,1])

n = y.size

D = sp.sparse.eye(n,n)
for i in range(0,n):
    for j in range(0,n):
        D[i,j] = y[i]*y[j]*(np.dot(P[:,i], P[:,j]) )

q = -1*np.ones(n)
b = 0
lb = np.zeros(n)

m = osqp.OSQP()

m.setup(P=D, q=q, A=y, l=lb)
results = m.solve()

print(results)
