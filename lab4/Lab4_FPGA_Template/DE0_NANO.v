`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

///////* DON'T CHANGE THIS PART *///////
module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY
);

//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;
localparam WHITE = 8'b11111111;

//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK - DON'T NEED TO CHANGE THIS //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:0]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;


assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];

///// I/O for Img Proc /////
wire [8:0] RESULT;

reg start = 0;

/* WRITE ENABLE */
reg W_EN = 1;

///////* CREATE ANY LOCAL WIRES YOU NEED FOR YOUR PLL *///////
wire c0_sig;
wire c1_sig;
wire c2_sig;
assign GPIO_0_D[32] = c0_sig;

wire D7,D6,D5,D4,D3,D2,D1,D0, PCLK,VSYNC, HREF;
assign PCLK = GPIO_1_D[28];
assign VSYNC = GPIO_1_D[26];
assign HREF = GPIO_1_D[24];
assign D7 = GPIO_1_D[22];
assign D6 = GPIO_1_D[20];
assign D5 = GPIO_1_D[18];
assign D4 = GPIO_1_D[16];
assign D3 = GPIO_1_D[14];
assign D2 = GPIO_1_D[12];
assign D1 = GPIO_1_D[10];
assign D0 = GPIO_1_D[8];


///////* INSTANTIATE YOUR PLL HERE *///////
sweetPLL	sweetPLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( c0_sig ), // 24 MHz
	.c1 ( c1_sig ),
	.c2 ( c2_sig )
	);
	
///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(CLOCK_50),
	.clk_R(c1_sig), // DO WE NEED TO READ SLOWER THAN WRITE??
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(c1_sig),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(c1_sig), // 25 MHz
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);


///////* Update Read Address *///////
//always @ (posedge PCLK) begin
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end


always @ (posedge PCLK) begin
	if(VSYNC) begin
			X_ADDR = 0;
	end
	else begin
		if(HREF) begin			
			if(start == 0) begin 
				pixel_data_RGB332[7:2] = {D0,D1,D2,D5,D6,D7};
				start = 1;
				W_EN = 0;
			end
			else begin
				pixel_data_RGB332[1:0] = {D3,D4};
				start = 0;
				X_ADDR = X_ADDR + 1;
				W_EN = 1;
			end				
		end
		else begin
			X_ADDR = 0;
		end
	end
end

always @ (negedge HREF or posedge VSYNC) begin
	if (VSYNC) begin
		Y_ADDR = 0;
	end
	else begin
		Y_ADDR = Y_ADDR + 1;
	end
end

//always @ (posedge PCLK) begin
//	if (X_ADDR > `SCREEN_WIDTH) begin
//		X_ADDR = 0;
//		Y_ADDR = Y_ADDR + 1;
//	end
//	else begin
//		if (Y_ADDR > `SCREEN_HEIGHT) begin
//			X_ADDR = 0;
//			Y_ADDR = 0;
//		end
//		else begin
//			if (!start) begin
//				pixel_data_RGB332[7:2] = 6'b000111;
//				start = 1;
//			end
//				pixel_data_RGB332[1:0] = 2'b11;
//				X_ADDR = X_ADDR + 1;
//				start = 0;
//		end
//	end
//end
	
endmodule 