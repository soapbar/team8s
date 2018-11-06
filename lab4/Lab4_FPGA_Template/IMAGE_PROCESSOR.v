`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	VGA_VSYNC_NEG,
	RESULT
);


//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;

output [8:0] RESULT;

reg [14:0] RED;
reg [14:0] BLUE;


always @ (posedge CLK) begin

	
		if (PIXEL_IN[7:6] > PIXEL_IN[1:0])
			RED = RED+1;
		else begin
			if (PIXEL_IN[7:6] < PIXEL_IN[1:0])
				BLUE = BLUE +1;
		end
	

	
end

assign RESULT = (RED > BLUE) ? 9'b111000000 : 9'b000000111;

//if (VGA_PIXEL_X == `SCREEN_WIDTH && VGA_PIXEL_Y == `SCREEN_HEIGHT) begin
//		
//		if (RED > BLUE)
//			assign RESULT = {1,1,1,0,0,0,0,0,0};
//		else
//			assign RESULT = {0,0,0,0,0,0,1,1,1};
//		
//end

endmodule