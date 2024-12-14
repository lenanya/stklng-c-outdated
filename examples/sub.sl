// go to program start
10 jmp 100;

// loop
15 push 1;
17 addi;
20 push 10;
30 icmp lt;
40 brn 200;
50 return;

// program start
100 push 0;
110 gosub 15;
120 jmp 1000;

// clear bool and comparison from the stack
200 pop;
210 pop;
220 jmp 15;

// end of program
1000 prstk;