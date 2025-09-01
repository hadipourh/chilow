def generate_round_constants(ROUND):
    rc = []
    for i in range(ROUND):
        c = (i + ( (2**i) << 4 )) << 32
        cc = "0x" + f"{c:016X}" + "UL"
        rc.append(cc)
    print(", ".join(rc))
def generate_round_constants_40(ROUND):
    rc = []
    for i in range(ROUND):
        c = ( (i + ( (2**i) << 4 )) << 32 )  + (1 << 63)
        cc = "0x" + f"{c:016X}" + "UL"
        rc.append(cc)
    print(", ".join(rc))


generate_round_constants(8)
generate_round_constants_40(8)
