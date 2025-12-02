module room;
    reg clk;
    reg [7:0] rooms;
    wire [3:0] count;
    wire [3:0] countdown0, countdown1, countdown2, countdown3;
    wire [3:0] countdown4, countdown5, countdown6, countdown7;
    wire [7:0] lightson;
    reg [1:0] x;
    reg [1:0] y;
    wire [2:0] sum;

    co uut(
        .clk(clk),
        .rooms(rooms),
        .count(count),
        .countdown0(countdown0),
        .countdown1(countdown1),
        .countdown2(countdown2),
        .countdown3(countdown3),
        .countdown4(countdown4),
        .countdown5(countdown5),
        .countdown6(countdown6),
        .countdown7(countdown7),
        .lightson(lightson),
        .x(x),
        .y(y),
        .sum(sum)
    );
	initial begin
		$dumpfile("room.vcd");
		$dumpvars(0,room);
	end
    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end

    initial begin
        $monitor("Time=%0t | Rooms=%b | Count=%d | CD: [%d %d %d %d %d %d %d %d],Lightson:%b, sum:%d", 
            $time, rooms, count,
            countdown0, countdown1, countdown2, countdown3,
            countdown4, countdown5, countdown6, countdown7,lightson,sum);
		x = 2'b00;
		y = 2'b10;
        rooms = 8'b00000000; #10;
        rooms = 8'b00010010; #10;
        rooms = 8'b11111111; #10;
        rooms = 8'b00000000; #10;
        #100;
        rooms = 8'b00000001;#100;
        $finish;
    end
endmodule

