package Tetris;
import java.util.Scanner;
import java.util.Random;
public class input // class that holds the input function
{

	public charimino take_input()
    {
	   boolean valid = false;
	   String input_string;
	   charimino chariminos = charimino.I;

	   Scanner scanObj = new Scanner(System.in);  // Create a Scanner object
	   System.out.print("What are the type?");
	   System.out.print("\n");

	   while (!valid) // take tetromino types and save an char vector
	   {
		   input_string = scanObj.nextLine();
		   valid = true;

		   Random rand = new Random();
		   
		   if (!((input_string.equals("I") || input_string.equals("i") || input_string.equals("ı")) || (input_string.equals("O") || input_string.equals("o")) || (input_string.equals("T") || input_string.equals("t")) || (input_string.equals("J") || input_string.equals("j")) || (input_string.equals("L") || input_string.equals("l")) || (input_string.equals("S") || input_string.equals("s")) || (input_string.equals("Z") || input_string.equals("z")) || (input_string.equals("R") || input_string.equals("r"))))
		   {
			   valid = false;
			   System.out.print("Please enter a valid tetromino ('I', 'O', 'T', 'J', 'L', 'S', 'Z', 'R' ).");
			   System.out.print("\n");
		   }
		   else
		   {
			   switch (input_string.charAt(0))
			   {

				   case 'O':
				   case 'o':
					   chariminos = charimino.O;
					   break;
				   case 'T':
				   case 't':
					   chariminos = charimino.T;
					   break;
				   case 'J':
				   case 'j':
					   chariminos = charimino.J;
					   break;
				   case 'L':
				   case 'l':
					   chariminos = charimino.L;
					   break;
				   case 'S':
				   case 's':
					   chariminos = charimino.S;
					   break;
				   case 'Z':
				   case 'z':
					   chariminos = charimino.Z;
					   break;
				   case 'R':
				   case 'r':
				   {
					   charimino[] arr = {charimino.I, charimino.O, charimino.T, charimino.J, charimino.L, charimino.S, charimino.Z};
					   chariminos = arr[rand.nextInt(7)];
					   break;
				   }
				   default:
					   chariminos = charimino.I;
					   break;
			   }
		   }
	   }

	   	//scanObj.close();

	   return chariminos;
   }
   
   
   public int take_number()
   {
	   boolean valid = false;
	   int num_of_trmn = 0;
	   String input_string;
	   Scanner scanObj = new Scanner(System.in);  // Create a Scanner object

	   
	   while (!valid) // take num_of_tetrominos until it's a natural number
	   {
		   num_of_trmn = 0;
		   input_string = scanObj.nextLine();
		   valid = true;
		   for (int i = 0; i < input_string.length(); ++i)
		   {
			   if (!((input_string.charAt(i) - '0') >= 0 && (input_string.charAt(i) - '0') <= 9))
			   {
				   num_of_trmn = 0;
				   break;
			   }
			   else
			   {
				   num_of_trmn = num_of_trmn * 10 + (input_string.charAt(i) - '0');
			   }
		   }
		   if (num_of_trmn == 0)
		   {
			   valid = false;
			   System.out.print("Please enter a valid number.");
			   System.out.print("\n");
		   }
	   }

	   	//scanObj.close();
	   return num_of_trmn;
   }

   public String take_direction()
   {
	   boolean valid = false;
	   String input_string = new String();
	   Scanner scanObj = new Scanner(System.in);  // Create a Scanner object

	   while (!valid) // take num_of_tetrominos until it's a natural number
	   {
		   input_string = scanObj.nextLine();
		   valid = true;
		   input_string = input_string.toLowerCase();
		   if (!input_string.equals("left") && !input_string.equals("l") && !input_string.equals("right") && !input_string.equals("r"))
		   {
			   valid = false;
			   System.out.print("Enter valid direction: ");
		   }
	   }

	   //scanObj.close();
	   return input_string;
   }
}






























