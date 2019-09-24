# Logulation

A fairly simple logic gate propagation simulator. This was written to experiment with queues and pointers in C. The python program builder.py constructs circuit component objects. Component.compile_structure recursively compiles the circuit as a network of basic logic gates, and returns a representation readable by readin.

## To Run

Import builder to construct a circuit and print its representation. Save as circuit.lgl (or change the filename in sim.c). Compile and run sim.c (main currently runs tests on the adder made by builder.py but the rest of the code is fully functional for any circuit)