#!/usr/bin/python3

total = 0
n = 0

with open("output.txt") as f:
    for line in f:
        if 'delay' not in line:
            continue

        total += int(line.split(':')[1].strip())
        n = n + 1

print(total / n)