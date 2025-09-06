import gurobipy as gp
from gurobipy import GRB
import time
import sys	

ROUNDS           = int(sys.argv[1])
STATE_SIZE       = 32
def Integral_Ckeck(ROUNDS, active_inputs):
    balanced_vectors_found=[]
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
    model.addConstr(s.sum(0, '*') == len(active_inputs))
    for i in active_inputs:
        model.addConstr(s[0, i] == 1)


    
    
    # Set objective equal to the Hamming weight of the last vector in the division trail.
    model.setObjective(c.sum(ROUNDS-1, '*'), GRB.MINIMIZE)
    
    # Save model for inspection.
    #model.write('Chk.lp')
    
    # Callback fuction that checks whether the lower bound exceeds 1 or not. If it does, terminate the solver.
    def checkLowerBound(model, where):
        if where == GRB.Callback.MIP:
            objbnd = model.cbGet(GRB.Callback.MIP_OBJBND)
            if objbnd > 1.99:
                print("objective bound = "+str(objbnd))
                model.terminate()
    
    time_start = time.time()
    unit_vectors_found = []
    while len(unit_vectors_found) < STATE_SIZE:
        # Solve the model, interrupting it when the lower bound exceeds 1.
        model.optimize(checkLowerBound)
        if model.status == GRB.OPTIMAL:
            if model.objVal > 1.5:
                #print("Value of objective is not 1. Is this even possible with the custom termination? An integral distinguisher exists.")
                break
            else:
                for i in range(STATE_SIZE):
                    if c[ROUNDS-1, i].x > 0.5:
                        #print("Found the " + str(i) + "'th unit vector")
                        unit_vectors_found.append(i)
                        #print("Found " + str(len(unit_vectors_found)) + " out of " + str(STATE_SIZE) + " unit vectors.")
                        model.addConstr(c[ROUNDS-1, i] == 0) # Set this unit vector to zero.
                        model.update()
                        break
        else:
            break
    
    unit_vectors_found.sort()
    if len(unit_vectors_found) < 32:
        for out_ind in range(32):
            if out_ind not in unit_vectors_found:
                balanced_vectors_found.append(out_ind)
    return balanced_vectors_found

print(Integral_Ckeck(4,[1,3,5,7,9,11,13,15]))
print(Integral_Ckeck(4,[10,14,17,19,21,23,25,29]))
