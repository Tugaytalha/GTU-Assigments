package Tetris;
import java.io.IOException;



public class Driver2
{

	public static void main(String[] args) {
		// Providing a seed value

		input input_object = new input();
		int wide;
		int high;
		boolean valid = true;

		System.out.print("Enter board wide: ");
		while ((wide = input_object.take_number()) < 10)
		{
			System.out.print("Enter a number greater than 10: ");
		}
		System.out.print("Enter board high: ");
		while ((high = input_object.take_number()) < 10)
		{
			System.out.print("Enter a number greater than 10: ");
		}
		Tetris game_board = new Tetris(wide, high);

		while (valid)
		{
			Tetromino Tet = new Tetromino(input_object.take_input());
			valid = (game_board.addAssignment(Tet));
			game_board.Rotate("r");
			if (valid)
			{
				game_board.animate();
			}
		}
		game_board.Draw();

		Runtime rt = Runtime.getRuntime();
		try {
			rt.exec("clear");
		} catch (IOException e) {
			System.out.println("Opsss!!!!");
		}
		System.out.print(" \033[15B");
		System.out.print("\033[40C");
		System.out.print("Game over!!!");
		System.out.print(" \033[10B");

	}
}