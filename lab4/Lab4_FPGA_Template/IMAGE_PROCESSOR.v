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

wire [8:0] x_coord;
wire [8:0] y_coord;
assign x_coord = pixel_count % 176;
assign y_coord = pixel_count / 144;

reg [9:0] RED_FRAMES;
reg [9:0] BLUE_FRAMES;

reg got_first;
reg [8:0] first_x;
reg [8:0] first_y;
reg [8:0] left_x;
reg [8:0] left_y;
reg [8:0] right_x;
reg [8:0] right_y;
reg [8:0] bottom_x;
reg [8:0] bottom_y;
wire [8:0] flx_diff;
assign flx_diff = first_x > left_x ? first_x - left_x : left_x - first_x;
wire [8:0] frx_diff;
assign frx_diff = first_x > right_x ? first_x - right_x : right_x - first_x;
wire [8:0] lby_diff;
assign lby_diff = left_y > bottom_y ? left_y - bottom_y : bottom_y - left_y;
wire [8:0] tbx_diff;
assign fbx_diff = first_x > bottom_x ? first_x - bottom_x : bottom_x - first_x;

reg [9:0] square_frames;
reg [9:0] diamond_frames;
reg [9:0] triangle_frames;
reg [9:0] none_frames;
reg [9:0] max_shape_frames;

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
		got_first = 0;
		left_x = 176;
		right_x = 0;
		bottom_y = 0;
	end
	
	// prevent color frame overflow
	if (RED_FRAMES == 10'b1111111111) begin
		RED_FRAMES = RED_FRAMES - BLUE_FRAMES;
		BLUE_FRAMES = 0;

	end
	if (BLUE_FRAMES == 10'b1111111111) begin
		BLUE_FRAMES = BLUE_FRAMES - RED_FRAMES;
		RED_FRAMES = 0;
	end
	
	// prevent shape frame overflow
	if (square_frames == 10'b1111111111 || triangle_frames == 10'b1111111111 ||
	    diamond_frames == 10'b1111111111 || none_frames == 10'b1111111111) begin
		if (square_frames > 0) begin
			square_frames = square_frames - 1;
		end
		if (triangle_frames > 0) begin
			triangle_frames = square_frames - 1;
		end
		if (diamond_frames > 0) begin
			diamond_frames = square_frames - 1;
		end
		if (none_frames > 0) begin
			none_frames = none - 1;
		end
	end
	
	// compare current pixel
	if (PIXEL_IN[7:6] > PIXEL_IN[1:0])
		RED_PIXELS = RED_PIXELS + 1;
	else begin
		if (PIXEL_IN[7:6] < PIXEL_IN[1:0])
			BLUE_PIXELS = BLUE_PIXELS + 1;
	end

	// find first, left and bottom
	if ((PIXEL_IN[7:6] == 2'b11 && PIXEL_IN[1:0] == 2'b00) || (PIXEL_IN[1:0] == 2'b11 && PIXEL_IN[7:6] == 2'b00)) begin
		if (~got_first) begin
			first_x = x_coord;
			first_y = y_coord;
			got_first = 1;
		end
		if (x_coord < left_x) begin
			left_x = x_coord;
			left_y = y_coord;
		end
		if (x_coord > right_x) begin
			right_x = x_coord;
			right_y = y_coord;
		end
		if (y_coord > bottom_y) begin
			bottom_x = x_coord;
			bottom_y = y_coord;
		end
	end
	
	// determine shape
	if (flx_diff < 25 || frx_diff < 25) begin
		square_frames = square_frames + 1;
	end
	else begin
		if (lby_diff < 25) begin
			triangle_frames = triangle_frames + 1;
		end
		else begin
			if (fbx_diff < 25) begin
				diamond_frames = diamond_frames + 1;
			end
			else 
				none_frames = none_frames + 1;
		end
	end

	// find max_shape_frames
	if (none_frames > square_frames && none_frames > triangle_frames && none_frames > diamond_frames) begin
		max_shape_frames = none_frames;
	end
	else begin 
		if (square_frames > none_frames && square_frames > triangle_frames && square_frames > diamond_frames) begin
			max_shape_frames = square_frames;
		end
		else begin
			if (triangle_frames > none_frames && triangle_frames > square_frames && triangle_frames > diamond_frames) begin
				max_shape_frames = triangle_frames;
			end
			else begin
				max_shape_frames = diamond_frames;
			end
		end
	end	
	
	pixel_count = pixel_count + 1;
end

assign RESULT[0] = (max_shape_frames == none_frames || max_shape_frames == triangle_frames) ? 0 : 1;
assign RESULT[1] = (max_shape_frames == none_frames || max_shape_frames == square_frames) ? 0 : 1;
assign RESULT[2] = (RED_FRAMES > BLUE_FRAMES) ? 0 : 1;
assign RESULT[8:3] = flx[5:0];
 

endmodule