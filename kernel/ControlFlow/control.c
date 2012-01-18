
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

   jmp_buf jumper;
   jmp_buf reset;

   int divide(int a, int b)
   {
      if (b == 0) // can't divide by 0
         longjmp(jumper, 3);
      return a / b;
   }

   void main(void)
   {

	   switch (setjmp(jumper)) {
	   case 3:
		   printf("Failed with divide by zero error\n");
		   break;
	   case 2:
		   printf("Failed with type char error\n");
		   break;
	   default:
		   break;
	   }

	   int num = 10;
	   int option = 0;
	   int denominator = 0;
	   char character;
	   switch (setjmp(reset)){
	   case -1:
		   printf("Goodbye\n");
		   break;
	   case 2:
		   printf("You entered an invalid choice! Please try again.");
		   longjmp(reset,1);
		   break;
	   default:
		   printf("Dividing %d by a number\n",num);
		   printf("Enter 1 to enter a number\nEnter 2 to enter a character\nEnter 3 to quit\nChoice: ");
		   scanf("%d",&option);
		   if(option == 1){
			   printf("Enter number: ");
		   	   scanf("%d",&denominator);
			   num = divide(num,denominator);
			   printf("The answer is %d\n",num);
			   longjmp(reset,0);
		   } else if(option == 2){
			   printf("Enter character: ");
		   	   scanf("%c",&character);
		   	   longjmp(jumper, 2);
		   } else if(option == 3){
			   longjmp(reset, -1);
		   } else {
			   longjmp(reset,2);
		   }
		   break;
   	   }
   }
