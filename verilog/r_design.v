module co (
    input [7:0] rooms,
    input clk,
    output reg [3:0] count,
    output reg [3:0] countdown0,
    output reg [3:0] countdown1,
    output reg [3:0] countdown2,
    output reg [3:0] countdown3,
    output reg [3:0] countdown4,
    output reg [3:0] countdown5,
    output reg [3:0] countdown6,
    output reg [3:0] countdown7,
    output reg [7:0] lightson,
    input [1:0]x,
    input [1:0]y,
    output reg[2:0] sum
);
    integer i;
    reg [3:0] countdown_array [7:0];  // internal array
    function [2:0] add;
    	input[1:0] x;
		input[1:0] y;
		begin
			add = x + y;
		end
	endfunction
    initial begin
        countdown_array[0] = 4'd0;
        countdown_array[1] = 4'd0;
        countdown_array[2] = 4'd0;
        countdown_array[3] = 4'd0;
        countdown_array[4] = 4'd0;
        countdown_array[5] = 4'd0;
        countdown_array[6] = 4'd0;
        countdown_array[7] = 4'd0;
        for (i = 0;i < 8;i = i+1) begin
        	lightson[i] = 0;
        end
    end

    always @(posedge clk) begin
        count = 4'd0;

        for (i = 0; i < 8; i = i + 1) begin
            if (rooms[i] == 1'b1) begin
                count = count + 1;
                lightson[i] = 1;
            end
        end
		
		for (i = 0;i < 8;i = i+1) begin
        	if (count<6 && rooms[i]==1'b1)begin
        		lightson[i] = 1;
        		countdown_array[i] = 4'd10;
        	end
        	else if(count>5 && rooms[i]==1'b1)begin
        		lightson[i] = 1;
        		countdown_array[i] = 4'd5;
        	end
        	else if((count>5 ||count<6)&&(rooms[i]==1'b0)&&countdown_array[i]>4'd0)begin
        		countdown_array[i] = countdown_array[i] -1;
        	end
        	
        	if(countdown_array[i]==4'd0)begin
        		lightson[i] = 0;
        	end
        end
		sum = add(x,y);
        // assign internal array to individual outputs
        countdown0 = countdown_array[0];
        countdown1 = countdown_array[1];
        countdown2 = countdown_array[2];
        countdown3 = countdown_array[3];
        countdown4 = countdown_array[4];
        countdown5 = countdown_array[5];
        countdown6 = countdown_array[6];
        countdown7 = countdown_array[7];
    end
endmodule

