package Tetris;
import java.util.*;



public class Driver1
{
	///#define NDEBUG


	public static void main(String[] args) {
		Random rand = new Random();

		Tetromino a = new Tetromino(charimino.S);
		System.out.print("Tetromino:");
		System.out.print("\n");
		a.print();
		System.out.print("Tetromino rotated left:");
		System.out.print("\n");
		a.rotate();
		a.print();
		System.out.print("Tetromino rotated left x2:");
		System.out.print("\n");
		a.rotate();
		a.print();

		Tetris sa = new Tetris(10, 20);
		String temp;
		System.out.print("Empty board:");
		System.out.print("\n");
		sa.Draw();

		Scanner scanObj = new Scanner(System.in);
		System.out.print("Press enter");
		temp = scanObj.nextLine();
		System.out.print("Tetromino added:");
		System.out.print("\n");
		sa.addAssignment(a);
		sa.Draw();
		System.out.print("Tetromino rotated right:");
		System.out.print("\n");
		sa.Rotate("r");
		sa.Draw();

		System.out.print("Press enter for fast random play");
		temp = scanObj.nextLine();
		Tetris as = new Tetris(10, 20);
		charimino[] arr = {charimino.I, charimino.O, charimino.T, charimino.J, charimino.L, charimino.S, charimino.Z};
		boolean flag = true;

		for (int i = 0; i < 120 && flag; i++)
		{
			Tetromino t = new Tetromino(arr[rand.nextInt(7)]);
			flag = as.addAssignment(t);
			int tt = rand.nextInt(4);
			for (i = 0; i < tt; ++i)
			{
				t.rotate();
			}
			while (as.move_test('d'))
			{
				as.logicalOr(0);
				as.rightShift(rand.nextInt(15) - 7);
				if (i % 8 == 0)
				{
					as.Draw();
				}
			}
		}
		as.Draw();
		System.out.print("Game finished, Press enter for next driver");
		temp = scanObj.nextLine();
		scanObj.close();


	}
}