import gurobipy as gp
from gurobipy import GRB
import time
import sys	

ROUNDS = int(sys.argv[1])
STATE_SIZE = 32

for ind1 in range(32):
	#print("ind1:"+str(ind1))
	for ind2 in range(ind1+1,32):	
		for ind3 in range(ind2+1,32):	
			for ind4 in range(ind3+1,32):	
				for input_indx in range(1):#range(STATE_SIZE):
					unit_vectors_found=[]
					#print("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@start new bit"+str(input_indx))
					env = gp.Env(empty=True)
					env.setParam("OutputFlag", 0)
					env.start()
					model = gp.Model('Chilow', env=env)
					#### Variables related to round function. ####
					# Indexing is as follows: (round, bit).
					# Input of round i.
					s = model.addVars(ROUNDS+1, STATE_SIZE, vtype=GRB.BINARY, name='s')   
					# Output of chi of round i.
					c = model.addVars(ROUNDS, STATE_SIZE, vtype=GRB.BINARY, name='c') 
					# Auxiliary variables for copies inside of chi and L of round i.
					u = model.addVars(ROUNDS, STATE_SIZE, vtype=GRB.BINARY, name='u')
					v = model.addVars(ROUNDS, STATE_SIZE, vtype=GRB.BINARY, name='v')
					w = model.addVars(ROUNDS, STATE_SIZE, vtype=GRB.BINARY, name='w')
					x = model.addVars(ROUNDS, STATE_SIZE, vtype=GRB.BINARY, name='x')
					y = model.addVars(ROUNDS, STATE_SIZE, vtype=GRB.BINARY, name='y')
					z = model.addVars(ROUNDS, STATE_SIZE, vtype=GRB.BINARY, name='z')
					# Auxiliary variables for AND gate inside of chi of round i.
					a = model.addVars(ROUNDS, STATE_SIZE, vtype=GRB.BINARY, name='a')
					
					#### Constraints. In general, we leave out the inverters, because they are equivalent to an XOR with a constant (1). #### 
					
					# Set round function constraints.  
					for i in range(ROUNDS):
						# Constraints for chi
						model.addConstrs(u[i, j] + v[i, j] + w[i, j]  == s[i, j] for j in range(STATE_SIZE)) # Copies.
						# Constraints for chi indices 0-12 and 17-29
						model.addConstrs(v[i, (j+1) % STATE_SIZE] + w[i, (j + 2) % STATE_SIZE] >= a[i, j] for j in range(13)) # AND gate.
						model.addConstrs(v[i, (j+1) % STATE_SIZE] + w[i, (j + 2) % STATE_SIZE] <= 2*a[i, j] for j in range(13)) # AND gate.
						model.addConstrs(u[i, j] + a[i, j]  == c[i, j] for j in range(13)) # XOR.
						model.addConstrs(v[i, (j+1) % STATE_SIZE] + w[i, (j + 2) % STATE_SIZE] >= a[i, j] for j in range(17,30)) # AND gate.
						model.addConstrs(v[i, (j+1) % STATE_SIZE] + w[i, (j + 2) % STATE_SIZE] <= 2*a[i, j] for j in range(17,30)) # AND gate.
						model.addConstrs(u[i, j] + a[i, j]  == c[i, j] for j in range(17,30)) # XOR.
						# Constraints for chi indices 13-16, 30, 31
						model.addConstr(v[i, 14] + w[i,0] >= a[i, 13])
						model.addConstr(v[i, 14] + w[i, 0] <= 2*a[i, 13])
						model.addConstr(u[i, 16] + a[i, 13]  == c[i, 13])
						    
						model.addConstr(v[i, 0] + w[i, 1] >= a[i, 14])
						model.addConstr(v[i, 0] + w[i, 1] <= 2*a[i, 14])
						model.addConstr(u[i, 15] + a[i, 14]  == c[i, 14])
						    
						model.addConstr(v[i, 16] + w[i, 17] >= a[i, 15])
						model.addConstr(v[i, 16] + w[i, 17] <= 2*a[i, 15])
						model.addConstr(u[i, 13] + a[i, 15]  == c[i, 15])
						    
						model.addConstr(v[i, 17] + w[i, 18] >= a[i, 16])
						model.addConstr(v[i, 17] + w[i, 18] <= 2*a[i, 16])
						model.addConstr(u[i, 14] + a[i, 16]  == c[i, 16])
						    
						model.addConstr(v[i, 31] + w[i, 15] >= a[i, 30])
						model.addConstr(v[i, 31] + w[i, 15] <= 2*a[i, 30])
						model.addConstr(u[i, 30] + a[i, 30]  == c[i, 30])
						    
						model.addConstr(v[i, 15] + w[i, 16] >= a[i, 31])
						model.addConstr(v[i, 15] + w[i, 16] <= 2*a[i, 31])
						model.addConstr(u[i, 31] + a[i, 31]  == c[i, 31])
						# Constraints for L
						model.addConstrs(x[i, j] + y[i, j] + z[i, j] == c[i, j] for j in range(STATE_SIZE)) # Copies.
						model.addConstrs(s[i+1, j] == x[i, (11*j+5)%STATE_SIZE] + y[i, (11*j+9)%STATE_SIZE] + z[i, (11*j+12)%STATE_SIZE] for j in range(STATE_SIZE)) # XOR.
					
					# Set input vector.
					#model.addConstrs(s[0, j] == 1 for j in range(STATE_SIZE-1))
					model.addConstr(s.sum(0, '*') == 4)
					model.addConstr(s[0, ind1] == 1)
					model.addConstr(s[0, ind2] == 1)
					model.addConstr(s[0, ind3] == 1)
					model.addConstr(s[0, ind4] == 1)
					    
					#model.addConstr(s[ROUNDS, 6] == 1)
					#model.addConstr(s.sum(ROUNDS, '*') == 1)
					model.setObjective(s.sum(ROUNDS, '*'), GRB.MINIMIZE)
					model.write('ChiLow.lp')
					
					while len(unit_vectors_found) < STATE_SIZE:
						model.optimize()
						#print(model.status)
						if model.status == GRB.OPTIMAL:
							if model.objVal > 1.5:
								break
							else:
								for i in range(STATE_SIZE):
		            						if s[ROUNDS, i].x > 0.5:
								                unit_vectors_found.append(i)
								                model.addConstr(s[ROUNDS, i] == 0) # Set this unit vector to zero.
								                model.update()
								                break
						else:
							break
					balanced_vectors_found=[]
					if len(unit_vectors_found) < 32:
						for out_ind in range(32):
							if out_ind not in unit_vectors_found:
								#print(out_ind)
								balanced_vectors_found.append(out_ind)
						print(str(balanced_vectors_found)+"***********ind1:"+str(ind1)+"  ind2:"+str(ind2)+"  ind3:"+str(ind3)+"  ind4:"+str(ind4))
