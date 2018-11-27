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
reg [8:0] flx_diff;
reg [8:0] frx_diff;
reg [8:0] lby_diff;
reg [8:0] fbx_diff;

reg [8:0] first_x_reg;

reg [9:0] square_frames;
wire [3:0] square_ones;
reg [9:0] diamond_frames;
wire [3:0] diamond_ones;
reg [9:0] triangle_frames;
wire [3:0] triangle_ones;
reg [9:0] none_frames;
wire [3:0] none_ones;

ONE_COUNT square_c (
	.number(square_frames),
	.ones(square_ones)
);

ONE_COUNT diamond_c (
	.number(diamond_frames),
	.ones(diamond_ones)
);

ONE_COUNT triangle_c (
	.number(triangle_frames),
	.ones(triangle_ones)
);

ONE_COUNT none_c (
	.number(none_frames),
	.ones(none_ones)
);

reg [9:0] prev_pixels; // for edge detection
wire [4:0] f;
wire [4:0] s;
wire [2:0] f_ones;
wire [2:0] s_ones;
assign f = prev_pixels[9:5];
assign s = prev_pixels[4:0];

ONE_COUNT5b f_c (
	.number(f),
	.ones(f_ones)
);

ONE_COUNT5b s_c (
	.number(s),
	.ones(s_ones)
);

always @ (posedge CLK) begin
	// end of frame
	if (pixel_count > 25344) begin
		first_x_reg = first_x;
		flx_diff = first_x > left_x ? first_x - left_x : left_x - first_x;
		frx_diff = first_x > right_x ? first_x - right_x : right_x - first_x;
		lby_diff = left_y > bottom_y ? left_y - bottom_y : bottom_y - left_y;
		fbx_diff = first_x > bottom_x ? first_x - bottom_x : bottom_x - first_x;
		// determine shape
		square_frames = square_frames << 1;
		triangle_frames = triangle_frames << 1;
		diamond_frames = diamond_frames << 1;
		none_frames = none_frames << 1;
		if (flx_diff < 5 || frx_diff < 5) begin
			square_frames = square_frames | 1;
		end
		else begin
			if (lby_diff < 5) begin
				triangle_frames = triangle_frames | 1;
			end
			else begin
				if (fbx_diff < 5) begin
					diamond_frames = diamond_frames | 1;
				end
				else 
					none_frames = none_frames | 1;
			end
		end
		
		if (RED_PIXELS > BLUE_PIXELS) 
			RED_FRAMES = RED_FRAMES + 1;
		if (BLUE_PIXELS > RED_PIXELS)
			BLUE_FRAMES = BLUE_FRAMES + 1;
			
		// prevent color frame overflow
		if (RED_FRAMES == 10'b1111111111) begin
			RED_FRAMES = RED_FRAMES - BLUE_FRAMES;
			BLUE_FRAMES = 0;

		end
		if (BLUE_FRAMES == 10'b1111111111) begin
			BLUE_FRAMES = BLUE_FRAMES - RED_FRAMES;
			RED_FRAMES = 0;
		end
	
		
		RED_PIXELS = 0;
		BLUE_PIXELS = 0;
		pixel_count = 0;
		got_first = 0;
		left_x = 176;
		right_x = 0;
		bottom_y = 0;
		prev_pixels = 0;
	end
	
	
	
	// compare current pixel
	if (PIXEL_IN[7:6] > PIXEL_IN[1:0])
		RED_PIXELS = RED_PIXELS + 1;
	else begin
		if (PIXEL_IN[7:6] < PIXEL_IN[1:0])
			BLUE_PIXELS = BLUE_PIXELS + 1;
	end

	// update stream
	if ((PIXEL_IN[7:6] == 2'b11 && PIXEL_IN[1:0] == 2'b00) || (PIXEL_IN[1:0] == 2'b11 && PIXEL_IN[7:6] == 2'b00)) begin
		prev_pixels = (prev_pixels << 1) | 1;
	end
	else begin
		prev_pixels = prev_pixels << 1;
	end
	
	// update regiters if edge
	if ((f_ones < 3) && (s_ones > 2)) begin
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
		
	
	pixel_count = pixel_count + 1;
end

assign RESULT[0] = (triangle_ones > 5 || diamond_ones > 5) ? 1 : 0;
assign RESULT[1] = (square_ones > 5 || diamond_ones > 5) ? 1 : 0;
assign RESULT[2] = (RED_FRAMES > BLUE_FRAMES) ? 0 : 1;
assign RESULT[8:3] = RED_FRAMES[5:0];


endmodule