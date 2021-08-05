import argparse

parser = argparse.ArgumentParser(description="Concatenates two files together.")
parser.add_argument("in1", help="First input file")
parser.add_argument("in2", help="Second input file")
parser.add_argument("out", help="Output file")
args = parser.parse_args()

# Open input files for reading and output file for writing
in1 = open(args.in1, "rb")
in2 = open(args.in2, "rb")
out = open(args.out, "wb")

# Write input to output
out.write(in1.read())
out.write(in2.read())

# Close files
in1.close()
in2.close()
out.close()