package Tetris;

import java.util.Scanner;



public class Tetris
{
	// Calls other constructor that takes wide and high with 10 wide and 20 high
	public Tetris()
	{
		this(10, 20);
	}

	// Makes tetris board
	public Tetris(int wide, int high)
	{
		board_high = high;
		board_wide = wide;

		board_vec = new char[board_high][board_wide];
		for (int i = 0; i < board_high; ++i)
		{
			for (int j = 0; j < board_wide; ++j)
			{
				board_vec[i][j] = ' ';
			}
		}
		last_loc = new Location[4];
		last_loc[0] = new Location();
		last_loc[1] = new Location();
		last_loc[2] = new Location();
		last_loc[3] = new Location();
	}


	// draw board to screen

	public void Draw()
	{

		for (int i = -1; i <= board_high; ++i)
		{
			System.out.print("x ");
			for (int j = 0; j < board_wide; ++j)
			{
				if (-1 == i || board_high == i)
				{
					System.out.print("x ");
				}
				else
				{
					System.out.print(board_vec[i][j]);
					System.out.print(" ");
				}
			}
			System.out.print("x ");
			System.out.print("\n");
		}
		System.out.print("\n");
		System.out.print("\n");
		System.out.print("\n");
	}

	// add a tetrimino to middle top of the board
	public boolean addAssignment(Tetromino Tetro)
	{
		int tet_wide = 4;
		boolean wide_flag = true;
		for (int j = 3; j > -1 && wide_flag; --j)
		{
			for (int i = 0; i < 4 && wide_flag; ++i)
			{
				if (Tetro.getItem(i)[j] != ' ')
				{
					wide_flag = false;
					++tet_wide;
				}
			}
			--tet_wide;
		}

		int ccounter;
		int i = board_wide / 2 - tet_wide / 2;
		int j = 0;
		int loc_counter = 0;
		boolean flag = false;
		boolean return_value = true;


		for (int vrow = 0; vrow < 4 && return_value; ++vrow)
		{
			ccounter = 0;
			if (flag)
			{
				++j;
				i = board_wide / 2 - tet_wide / 2;
				flag = false;
			}
			for (int c = 0; c < 4 && return_value; ++c)
			{
				if (!flag)
				{ // until current on tetromino letter
					if (Tetro.getItem(vrow)[c] != ' ')
					{
						flag = true;
						for (int k = ccounter; k > 0; --k)
						{
							//last_tetromino[j][i] = ' ';
							board_vec[j][i++] = ' ';
						}
						last_loc[loc_counter].moved = false;
						last_loc[loc_counter].y = i;
						last_loc[loc_counter++].x = j;

						board_vec[j][i] = Tetro.getItem(vrow)[c];
						++i;
					}
				}
				else
				{
					if (' ' == board_vec[j][i])
					{
						//last_tetromino[j][i] = Tetro.getItem(vrow)[c];
						if (Tetro.getItem(vrow)[c] != ' ')
						{
							last_loc[loc_counter].moved = false;
							last_loc[loc_counter].y = i;
							last_loc[loc_counter++].x = j;
							board_vec[j][i] = Tetro.getItem(vrow)[c];
							++i;
						}
					}
					else if (' ' != Tetro.getItem(vrow)[c])
					{
						return_value = false;
					}
				}
				++ccounter;
			}
		}

		return return_value;
	}
	public final boolean animate()
	{
		String rotate_string;
		String move_string;
		int rotate_no;
		int move_no;
		boolean valid = true;
		Scanner scanObj = new Scanner(System.in);
		input inputObj = new input();
		//step 1
		Draw();
		
		
		//step 2
		System.out.println("rotate Which direction?");
		rotate_string = scanObj.nextLine();
		System.out.println("rotate How many times?");
		rotate_no = inputObj.take_number();
		//step 3
		System.out.println("move Which direction ");
		move_string = scanObj.nextLine();
		System.out.println("move How many times? ");
		move_no = inputObj.take_number();
		//scanObj.close();

		//step 4
		move_no *= (move_string.toLowerCase().equals("l") || move_string.toLowerCase().equals("left")) ? - 1 : 1;
		if (rotate_string.toLowerCase().equals("l") || rotate_string.toLowerCase().equals("left"))
		{
			rotate_string = "r";
			rotate_no *= 3;
		}

		for (rotate_no %= 4; rotate_no > 0 && (valid = move_test((rotate_string.equals("l") || rotate_string.equals("left")) ? '1' : '2')); --rotate_no)
		{
			Rotate(rotate_string);
		}

		if (!valid)
		{
			return false;
		}
		if (!(this.rightShift(move_no)))
		{
			System.out.print("Couldn't as shift as want beacuse of impossible");
			System.out.print("\n");
		}


		while (move_test('d'))
		{
			//step 5
			Draw();
			//step 6
			try {
				Thread.sleep(50);
			} catch(InterruptedException e) {
				System.out.println("got interrupted by slepp !!?");
			}
			//step 7
			this.logicalOr(0);

		}
		Draw();

		return true;
	}

	// move tetromino horizantal in given number(negatives represent left)
	public boolean rightShift(int number)
	{
		boolean return_value = false;
		if (number < 0)
		{
			for (int i = 0; i > number && (return_value = (move_test('l'))) ; --i)
			{
				move_left(0);
			}
		}
		else
		{
			for (int i = 0; i < number && (return_value = move_test('r')); ++i)
			{
				move_right(0);
			}
		}

		return return_value;
	}

	//moves the tetromino left a notch
	public final void move_left(int index)
	{
		boolean inner_recursive = false;
		if (index < 0)
		{
			inner_recursive = true;
			index *= -1;
		}

		if (false == last_loc[index].moved)
		{
			for (int j = 0; j < 4; ++j)
			{ // if any other piece in the right move it first
				if (((last_loc[index].lessThan(last_loc[j]))))
				{
					move_left(-j);
				}
			}
			board_vec[last_loc[index].x][last_loc[index].y - 1] = board_vec[last_loc[index].x][last_loc[index].y];
			board_vec[last_loc[index].x][last_loc[index].y] = ' ';
			--last_loc[index].y;
			last_loc[index].moved = true;
		}
		if (index != 3 && !inner_recursive)
		{
			move_left(index + 1);
		}
		else if (!inner_recursive)
		{ // if this is last call reset all moved
			last_loc[0].moved = last_loc[1].moved = last_loc[2].moved = last_loc[3].moved = false;
		}

	}

	//moves the tetromino right a notch
	public final void move_right(int index)
	{
		boolean inner_recursive = false;
		if (index < 0)
		{
			inner_recursive = true;
			index *= -1;
		}

		if (false == last_loc[index].moved)
		{
			for (int j = 0; j < 4; ++j)
			{ // if any other piece in the right move it first
				if (((last_loc[index].greaterThan(last_loc[j]))))
				{
					move_right(-j);
				}
			}
			board_vec[last_loc[index].x][last_loc[index].y + 1] = board_vec[last_loc[index].x][last_loc[index].y];
			board_vec[last_loc[index].x][last_loc[index].y] = ' ';
			++last_loc[index].y;
			last_loc[index].moved = true;
		}
		if (index != 3 && !inner_recursive)
		{
			move_right(index + 1);
		}
		else if (!inner_recursive)
		{ // if this is last call reset all moved
			last_loc[0].moved = last_loc[1].moved = last_loc[2].moved = last_loc[3].moved = false;
		}

	}

	// moves the tetromino down a notch
	public void logicalOr(int index)
	{
		boolean inner_recursive = false;
		if (index < 0)
		{
			inner_recursive = true;
			index *= -1;
		}

		if (false == last_loc[index].moved)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (((last_loc[index].greaterThanOrEqualTo(last_loc[j]))))
				{
					this.logicalOr(-j);
				}
			}
			board_vec[last_loc[index].x + 1][last_loc[index].y] = board_vec[last_loc[index].x][last_loc[index].y];
			board_vec[last_loc[index].x][last_loc[index].y] = ' ';
			++last_loc[index].x;
			last_loc[index].moved = true;
		}
		if (index != 3 && !inner_recursive)
		{
			this.logicalOr((index + 1));
		}
		else if (!inner_recursive)
		{ // if this is last call reset all moved
			last_loc[0].moved = last_loc[1].moved = last_loc[2].moved = last_loc[3].moved = false;
		}
	}

	// test tetromino movebility in given direction
//C++ TO JAVA CONVERTER WARNING: 'const' methods are not available in Java:
//ORIGINAL LINE: boolean move_test(sbyte LorRorD) const
	public final boolean move_test(char LorRorD)
	{
		boolean can_move = true;

		if (LorRorD == 'l')
		{ // can move to left
			for (int i = 0; i < 4 && can_move; ++i)
			{
				if (last_loc[i].y - 1 <= 0)
				{
					can_move = false;
				}
				else if (board_vec[last_loc[i].x][last_loc[i].y - 1] != ' ')
				{
					can_move = false;
					for (int j = 0; j < 4 && !can_move; ++j)
					{
						if ((i != j && (last_loc[i].lessThan(last_loc[j]))))
						{
							can_move = true;
						}
					}
				}
			}
		}
		else if (LorRorD == 'r')
		{ // can move to right
			for (int i = 0; i < 4 && can_move; ++i)
			{
				if (last_loc[i].y + 1 >= board_wide)
				{
					can_move = false;
				}
				else if (board_vec[last_loc[i].x][last_loc[i].y + 1] != ' ')
				{
					can_move = false;
					for (int j = 0; j < 4 && !can_move; ++j)
					{
						if ((i != j && (last_loc[i].greaterThan(last_loc[j]))))
						{
							can_move = true;
						}
					}
				}
			}
		}
		else if (LorRorD == 'd')
		{ // can move to down
			for (int i = 0; i < 4 && can_move; ++i)
			{
				if (last_loc[i].x + 1 >= board_high)
				{
					can_move = false;
				}
				else if (board_vec[last_loc[i].x + 1][last_loc[i].y] != ' ')
				{
					can_move = false;
					for (int j = 0; j < 4 && !can_move; ++j)
					{
						if ((i != j && (last_loc[i].greaterThanOrEqualTo(last_loc[j]))))
						{
							can_move = true;
						}
					}
				}
			}
		}
		else if (LorRorD == '1')
		{ // can rotate to left
			for (int index = 0; index < 4 && can_move; ++index)
			{
				int i;
				int j;
				int diffx;
				//int diffy;
				int maxx = Integer.MIN_VALUE;
				int maxy = Integer.MIN_VALUE;
				int minx = Integer.MAX_VALUE;
				int miny = Integer.MAX_VALUE;
				for (int f = 0; f < 4; ++f)
				{
					if (minx > last_loc[f].x)
					{
						minx = last_loc[f].x;
					}
					else if (maxx < last_loc[f].x)
					{
						maxx = last_loc[f].x;
					}
					if (miny > last_loc[f].y)
					{
						miny = last_loc[f].y;
					}
					else if (maxy < last_loc[f].y)
					{
						maxy = last_loc[f].y;
					}
				}

				i = last_loc[index].x - minx;
				j = last_loc[index].y - miny;
				diffx = maxx - minx;
				//diffy = maxy - miny;
				if (diffx - j + minx >= board_high || (i + miny) >= board_wide ||(i + miny) <= 0)
				{
					can_move = false;
				}
				else if (board_vec[diffx - j + minx][(i + miny)] != ' ')
				{
					can_move = false;
					for (int index2 = 0; index2 < 4 && !can_move; ++index2)
					{
						if (index != index2 && diffx - j + minx == last_loc[index2].x && i + miny == last_loc[index2].y)
						{
							can_move = true;
						}
					}
				}
			}
		}
		else if (LorRorD == '2')
		{ // can rotate to right
			for (int index = 0; index < 4 && can_move; ++index)
			{
				int i;
				int j;
				//int diffx;
				int diffy;
				int maxx = Integer.MIN_VALUE;
				int maxy = Integer.MIN_VALUE;
				int minx = Integer.MAX_VALUE;
				int miny = Integer.MAX_VALUE;
				for (int f = 0; f < 4; ++f)
				{
					if (minx > last_loc[f].x)
					{
						minx = last_loc[f].x;
					}
					if (maxx < last_loc[f].x)
					{
						maxx = last_loc[f].x;
					}
					if (miny > last_loc[f].y)
					{
						miny = last_loc[f].y;
					}
					if (maxy < last_loc[f].y)
					{
						maxy = last_loc[f].y;
					}
				}

				i = last_loc[index].x - minx;
				j = last_loc[index].y - miny;
				//diffx = maxx - minx;
				diffy = maxy - miny;
				if (j + minx >= board_high || ((diffy - i) + miny) >= board_wide ||((diffy - i) + miny) < 0)
				{
					can_move = false;
				}
				else if (board_vec[j + minx][((diffy - i) + miny)] != ' ')
				{
					can_move = false;
					for (int index2 = 0; index2 < 4 && !can_move; ++index2)
					{
						if (j + minx == last_loc[index2].x && (diffy - i) + miny == last_loc[index2].y)
						{
							can_move = true;
						}
					}
				}
			}
		}

		return can_move;
	}
	public final boolean Rotate(String LorR)
	{
		if (LorR.toLowerCase().equals("l") || LorR.toLowerCase().equals("left"))
		{
			Rotate_right(0, Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MIN_VALUE, Integer.MIN_VALUE);
			Rotate_right(0, Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MIN_VALUE, Integer.MIN_VALUE);
			return Rotate_right(0, Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MIN_VALUE, Integer.MIN_VALUE);
		}
		else
		{
			return Rotate_right(0, Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MIN_VALUE, Integer.MIN_VALUE);
		}


	}

	//recursive function that rotate last added tetromino on the board to right
	private boolean Rotate_right(int index, int minx, int miny, int maxx, int maxy)
	{
		boolean inner_recursive = false;
		boolean ok = true;
		if (index < 0)
		{
			inner_recursive = true;
			index *= -1;
		}
		if (false == last_loc[index].moved)
		{
			int i;
			int j;
			//int diffx;
			int diffy;
			for (int f = 0; f < 4 && !last_loc[f].moved; ++f)
			{
				if (minx > last_loc[f].x)
				{
					minx = last_loc[f].x;
				}
				if (maxx < last_loc[f].x)
				{
					maxx = last_loc[f].x;
				}
				if (miny > last_loc[f].y)
				{
					miny = last_loc[f].y;
				}
				if (maxy < last_loc[f].y)
				{
					maxy = last_loc[f].y;
				}
			}

			i = last_loc[index].x - minx;
			j = last_loc[index].y - miny;
			//diffx = maxx - minx;
			diffy = maxy - miny;

			
			if ((j + minx >= board_high) || (((diffy - i) + miny) >= board_wide) ||(((diffy - i) + miny) < 0))
			{
				return (false);
			}
			else if (j + minx == last_loc[index].x && (diffy - i) + miny == last_loc[index].y)
			{
				last_loc[index].moved = true;
				ok = false;
			}
			else if (board_vec[j + minx][(diffy - i) + miny] != ' ')
			{
				ok = false;
				for (int index2 = 0; index2 < 4; ++index2)
				{
					if (index != index2 && j + minx == last_loc[index2].x && (diffy - i) + miny == last_loc[index2].y)
					{
						if (index2 == 0)
						{
							last_loc[0].moved = true;
							last_loc[1].moved = true;
							last_loc[2].moved = true;
							last_loc[3].moved = true;
							ok = false;
						}
						else
						{
						Rotate_right(-index2, minx, miny, maxx, maxy);
						ok = true;
						}
					}
				}
			}

			if (ok)
			{
				if (last_loc[index].moved == false)
				{
				board_vec[j + minx][(diffy - i) + miny] = board_vec[i + minx][j + miny];
				board_vec[i + minx][j + miny] = ' ';
				last_loc[index].x = j + minx;
				last_loc[index].y = (diffy - i) + miny;
				last_loc[index].moved = true;
				}
			}
		}

		if (index != 3 && !inner_recursive)
		{
			Rotate_right(index + 1, minx, miny, maxx, maxy);
		}
		else if (!inner_recursive)
		{ // if this is last call reset all moved
			last_loc[0].moved = last_loc[1].moved = last_loc[2].moved = last_loc[3].moved = false;
		}
		return true;
	}
	private char[][] board_vec; // board array
	private Location[] last_loc; // last added tetromino's location
	private int board_wide;
	private int board_high;
}
