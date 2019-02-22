print "enter a number:"
n = io.read("*number")
for i = 2,n
do
	io.write(i)
	if n%i==0 then
		io.write(" is a factor of ")
	else
		 io.write(" is NOT a factor of ")
	end
	print(n)

end
