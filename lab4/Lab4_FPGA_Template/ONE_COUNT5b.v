module ONE_COUNT5b(
	number,
	ones
);

input [4:0] number;

output reg [2:0] ones;

integer i;
always @ (number)
begin
	ones = 0;
	for (i=0; i<5; i=i+1)
		ones = ones + number[i];
end

endmodule