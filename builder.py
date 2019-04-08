import types

class Component():

	# Maximum number of gate inputs and outputs is 3 in current sim.c implementation
	gate_in_num = 3
	gate_out_num = 3

	supported_gate_types = ['A', 'O', 'X', 'N', 'B']

	def __init__(self, name, in_labels, out_labels):
		self.name = name
		self.in_labels = [] + in_labels
		self.out_labels = [] + out_labels
		self.n_nodes = 0
		self.n_subcomponents = 0
		self.node_labels = {}

		self.add_labels(self.in_labels)

		self.gates = []
		# An item in this list is (<gate char>, (three input labels - empty string denotes unused input), (three output labels))
		self.subcomponents = []
		# Items are ('prefix for this component', Component instance)

		self.flattened = None

	def add_labels(self, x):
		if not isinstance(x, types.StringType):
			for l in x:
				# will not add empty string as label - that's used to denote empty input/output
				if l and l not in self.node_labels.keys():
					self.node_labels[l] = self.n_nodes
					self.n_nodes += 1
		else:
			if x and x not in self.node_labels.keys():
				self.node_labels[x] = self.n_nodes
				self.n_nodes += 1

	def add_gate(self, gate_type, ils, ols):
		assert gate_type in self.supported_gate_types
		self.add_labels(ils)
		self.add_labels(ols)
		self.gates.append((gate_type, ils, ols))

	def add_subcomponent(self, sub_name, subcomponent):
		prefix = sub_name + ':'
		self.add_labels([prefix + il for il in subcomponent.in_labels])
		self.add_labels([prefix + ol for ol in subcomponent.out_labels])

		self.subcomponents.append((prefix, subcomponent))

	def sanitize_linking(self):
		# Recall that in the C program one node can only be the input to one gate. 
		# Identify failures to follow this rule and solve by either:
		# - Find outputting gate and route through up to three separate nodes by amending the output locations
		# - Ignore outputting gate, instead use an or to branch to three new nodes
		assert not self.subcomponents # Only sanitize when structure is already flat

	def flatten(self):
		# returns a Component instance equivalent to itself but which is made up of only gates (no subcomponents)
		if not self.subcomponents:
			# Reconstruct self as-is
			out = Component(self.name, self.in_labels, self.out_labels)
			out.add_labels(self.node_labels.keys())
			for g in self.gates:
				out.add_gate(*g)
			return out
		else:
			print 'aaa'



	def compile_structure(self):
		if not self.flattened:
			self.flattened = self.flatten()
			self.flattened.sanitize_linking()

		out = ""
		out += "CIRCUIT " + self.name + "\n"
		out += "NODES: " + str(self.flattened.n_nodes) + "\n"
		out += "GATES: " + str(len(self.flattened.gates)) + "\n"

		def str_gate(f, g):
			o = g[0]
			try:
				for j in range(f.gate_in_num):
					if g[1][j]:
						try:
							o += ' '
							o += str(f.node_labels[g[1][j]])
						except KeyError:
							print "Gate with undefined label"
							print g
							raise KeyError()
					else:
						o += ' X'
				o += ' :'
				for j in range(f.gate_out_num):
					if gate[2][j]:
						try:
							o += ' '
							o += str(f.node_labels[g[2][j]])
						except KeyError:
							print "Gate with undefined label"
							print g
							raise KeyError()
					else:
						o += ' X'
			except IndexError:
				print "Misdefined gate"
				print g
				raise IndexError()
			return o
		print self.flattened.node_labels
		for i in range(len(self.flattened.gates)):
			gate = self.flattened.gates[i]
			print gate
			out += str_gate(self.flattened, gate) + "\n"

		return out



"""
remember to automatically sort out multiple references to the same output and toss those through a branching OR
remember that in the c simulation a single node can only have 1 dependent
(but one gate can have up to 3 outputs)
"""

"""
Output file format:
CIRCUIT <circuit name>
NODES: XX
GATES: XX
A 1 2 X : 3 X X
O 2 6 0 : 1 7 4
and so on
shouldn't really be too hard - the main pain in the ass is building up all the higher logic from scratch
"""

"""
Seperately this should provide a file of node #s and labels for the C program to display bits of interest with
"""

"""
First big goal: ALU. Next registers, next work with C system to incorporate memory modules, next the full processor
"""

"""
Finally, use python to build spatial maps that a C program will use to visualize circuit activity
"""

def main():
	adder = Component("ADDER", ["A", "B", "C"], ["Sum", "Carry"])
	adder.add_gate('O', ('A', '', ''), ('4', '5', ''))
	adder.add_gate('O', ('B', '', ''), ('6', '7', ''))
	adder.add_gate('O', ('C', '', ''), ('8', '9', ''))
	adder.add_gate('O', ('10', '', ''), ('11', '12', ''))
	adder.add_gate('X', ('4', '6', ''), ('10', '', ''))
	adder.add_gate('X', ('11', '8', ''), ('Sum', '', ''))
	adder.add_gate('A', ('5', '7', ''), ('14', '', ''))
	adder.add_gate('A', ('12', '9', ''), ('13', '', ''))
	adder.add_gate('O', ('13', '14', ''), ('Carry', '', ''))

	print adder.node_labels
	print adder.compile_structure()
	print adder.node_labels

if __name__ == '__main__':
	main()