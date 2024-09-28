# there must be a better way to do this

to_upper_code_points = [(c, c - ord(chr(c).upper())) for c in range(0x11000) if chr(c).isalpha() and len(chr(c).upper()) == 1 and chr(c)!= chr(c).upper()]
to_upper_ranges = [[to_upper_code_points[0][0], to_upper_code_points[0][0], to_upper_code_points[0][1]]]

for c in to_upper_code_points[1:]:
    if c[0] - to_upper_ranges[-1][1] == 1 and c[1] == to_upper_ranges[-1][2]:
        to_upper_ranges[-1][1] = c[0]
    else:
        to_upper_ranges.append([c[0], c[0], c[1]])

print("char32_t c32_to_upper(char32_t c) {")
print("    switch (c) {")
for r in to_upper_ranges:
    print(f"        case {hex(r[0])}", end = "")
    if r[0] != r[1]:
        print(f" ... {hex(r[1])}", end = "")
    print(f": return c {'-' if r[2] >= 0 else '+'} {hex(abs(r[2]))};")
print("        default: return c;")
print("    }")
print("}")

"""
is_alpha_code_points = [c for c in range(0x11000) if chr(c).isalpha() and len(chr(c)) == 1]
is_alpha_ranges = [[is_alpha_code_points[0], is_alpha_code_points[0]]]

for c in is_alpha_code_points[1:]:
    if c - is_alpha_ranges[-1][1] == 1:
        is_alpha_ranges[-1][1] = c
    else:
        is_alpha_ranges.append([c, c])

print("")
print("bool c32_is_alpha(char32_t c) {")
print("    return (")
first = True
for r in is_alpha_ranges:
    if first:
        first = False
    else:
        print(" ||")
    print("        ", end="")
    if r[0] == r[1]:
        print(f"с == {hex(r[0])}", end="")
    else:
        print(f"({hex(r[0])} <= c && c <= {hex(r[1])})", end="")
print("\n    )")
print("}")"""