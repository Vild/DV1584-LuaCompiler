print "enter a number:"
n = io.read("*number")
print(n)
factorial = 1
x=1
print(x)
for i = 2,n do
	 x = x * i
	 io.write("x is: ")
	 print(x)
end
io.write("factorial of ")
io.write(n)
io.write(" is ")
print(x)
