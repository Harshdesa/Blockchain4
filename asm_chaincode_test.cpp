using namespace std;
int main()
{
int val1,val2, add, sub, mul;

float valx, valy, result;

valx = 7.4;
valy = 5.4;

val1=100;val2=20;
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

printf( "%f cmp %f = %f\n", valx, valy, result );

return 0;
}
