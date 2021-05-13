#include<bits/stdc++.h>
using namespace std;
int main()
{
int val1,val2,val3,secondmax = 0,max,add, sub, mul;
int maximum, secondmaximum;
float valx, valy, result;

valx = 7.4;
valy = 5.4;

val1=130;val2=120; val3 = 50;
asm(
"FLDS %1 \n"
"FLDS %2 \n"
"FUCOMI %%st(1), %%st \n"
"FCMOVB %%st(1), %%st \n"
"FSTP %0 \n"
: "=m"(result)
: "m"(valx), "m"(valy)
:
);
asm(
"clc \n"
"movl %%eax, %1 \n"      // Current value
"movl %%ebx, %2 \n"      // Current max
"cmp %%ebx, %%eax \n"    // Compare value and max
"cmovb %%ebx, %%eax \n"     // if value is greater(EAX GREATED THAN EBX), %0 IS second max(MOV HAPPENS FROM EBX to %0)
"movl %0, %%ebx \n"
: "=r"(max)
: "a"(val1), "b"(val2)
);


asm(
"clc \n"
"movl %1, %%eax \n"
"movl %2, %%ebx \n"
"movl %3, %%ecx \n"
"movl %4, %%edx \n"
"cmp %%eax, %%ebx \n"
"cmovne %%ebx, %%ecx \n"
"clc \n"
"cmp %%edx, %%ecx \n"
"cmovb %%edx, %%ecx \n"
"movl %%ecx, %0 \n"
: "=r"(secondmax)
: "g"(max), "g"(val2), "g"(val3), "g"(val1)
);

asm(
"clc \n"
"movl %1, %%eax \n"
"movl %2, %%ebx \n"
"movl %3, %%ecx \n"
"cmp %%eax, %%ebx \n"
"cmove %%ecx, %%ebx \n"
"movl %%ebx, %0 \n"
: "=r"(secondmax)
:"g"(max), "g"(secondmax), "g"(val2)
);



//asm(
//"clc \n"
//"movl %%eax, %1 \n"
//"movl %%ebx, %2 \n"
//"movl %%ecx, %3 \n"
//"cmp %%eax, %%ebx \n"
//"cmovne %%ecx, %%ebx \n"
//"movl %0, %%ecx \n"
//: "=r"(secondmax)
//: "a"(max), "b"(val2), "c"(val3)
//);



printf("%d cmp %d\n", val1, val2);
printf("max %d\n", max);
printf("secondmax %d\n", secondmax);
asm( "addl %%ebx, %%eax;" : "=a" (add) : "a" (val1) , "b" (val2) );
asm( "subl %%ebx, %%eax;" : "=a" (sub) : "a" (val1) , "b" (val2) );
asm( "imull %%ebx, %%eax;" : "=a" (mul) : "a" (val1) , "b" (val2) );
printf( "%d + %d = %d\n", val1, val2, add );
printf( "%d - %d = %d\n", val1,val2, sub );
printf( "%d * %d = %d\n", val1, val2, mul );
printf( "%f cmp %f = %f\n", valx, valy, result );

return 0;
}
