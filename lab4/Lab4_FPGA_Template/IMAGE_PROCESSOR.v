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

wire [2:0] RED;
wire [2:0] GREEN;
wire [1:0] BLUE;

assign RED = PIXEL_IN[7:5];
assign GREEN = PIXEL_IN[4:2];
assign BLUE = PIXEL_IN[1:0];

reg [14:0] RED_PIXELS;
reg [14:0] BLUE_PIXELS;
reg [14:0] pixel_count;

reg [9:0] RED_FRAMES;
reg [9:0] BLUE_FRAMES;
reg [9:0] NONE_FRAMES;

wire [8:0] x_coord;
wire [8:0] y_coord;
assign x_coord = pixel_count % `SCREEN_WIDTH;
assign y_coord = pixel_count / `SCREEN_HEIGHT;

always @ (posedge CLK) begin
	// reset if end of frame
	if (pixel_count > 25344) begin
//		if (RED_PIXELS > BLUE_PIXELS + 3000)
//			RED_FRAMES = RED_FRAMES + 1;
//		else
		if (BLUE_PIXELS > 1000)
			BLUE_FRAMES = BLUE_FRAMES + 1;
		else 
			NONE_FRAMES = NONE_FRAMES + 1;
		RED_PIXELS = 0;
		BLUE_PIXELS = 0;
		pixel_count = 0;
	end
	
	// prevent frame overflow
	if (RED_FRAMES == 10'b1111111111 ||
	    BLUE_FRAMES == 10'b1111111111 ||
		 NONE_FRAMES == 10'b1111111111) begin
		if (RED_FRAMES > 0)
			RED_FRAMES = RED_FRAMES - 1;
		if (BLUE_FRAMES > 0)
			BLUE_FRAMES = BLUE_FRAMES - 1;
		if (NONE_FRAMES > 0)
			NONE_FRAMES = NONE_FRAMES - 1;
	end
	
	// check current pixel
	if (RED >= 3'b101 && GREEN <= 3'b011 && BLUE <= 2'b10)
		RED_PIXELS = RED_PIXELS + 1;
	else
		if (RED <= 3'b011 && GREEN <= 3'b011 && BLUE >= 2'b10)
			BLUE_PIXELS = BLUE_PIXELS + 1;

			
	pixel_count = pixel_count + 1;
end

assign RESULT[0] = RED_FRAMES > BLUE_FRAMES ? 0 : 1;
assign RESULT[1] = (NONE_FRAMES > RED_FRAMES && NONE_FRAMES > BLUE_FRAMES) ? 1 : 0;


endmodule