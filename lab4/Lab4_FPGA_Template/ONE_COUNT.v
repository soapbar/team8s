module ONE_COUNT(
	number,
	ones
);

input [9:0] number;

output reg [3:0] ones;

integer i;
always @ (number)
begin
	ones = 0;
	for (i=0; i<10; i=i+1)
		ones = ones + number[i];
end

endmodule