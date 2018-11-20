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

reg [14:0] RED_PIXELS;
reg [14:0] BLUE_PIXELS;
reg [14:0] pixel_count;

reg RED_FRAMES;
reg BLUE_FRAMES;


always @ (posedge CLK) begin
	// reset if end of frame
	if (pixel_count > 25344) begin
		if (RED_PIXELS > BLUE_PIXELS) 
			RED_FRAMES = RED_FRAMES + 1;
		if (BLUE_PIXELS > RED_PIXELS)
			BLUE_FRAMES = BLUE_FRAMES + 1;
		RED_PIXELS = 0;
		BLUE_PIXELS = 0;
		pixel_count = 0;
	end
	
	// prevent frame overflow
	if (RED_FRAMES == 15'b111111111111111) begin
		RED_FRAMES = RED_FRAMES - BLUE_FRAMES;
		BLUE_FRAMES = 0;
	end
	if (BLUE_FRAMES == 15'b111111111111111) begin
		BLUE_FRAMES = BLUE_FRAMES - RED_FRAMES;
		RED_FRAMES = 0;
	end
	
	// compare current pixel
	if (PIXEL_IN[7:6] > PIXEL_IN[1:0])
		RED_PIXELS = RED_PIXELS + 1;
	else begin
		if (PIXEL_IN[7:6] < PIXEL_IN[1:0])
			BLUE_PIXELS = BLUE_PIXELS + 1;
	end


	pixel_count = pixel_count + 1;
end

assign RESULT = (RED_FRAMES > BLUE_FRAMES) ? 9'b111000000 : 9'b000000111;

endmodule