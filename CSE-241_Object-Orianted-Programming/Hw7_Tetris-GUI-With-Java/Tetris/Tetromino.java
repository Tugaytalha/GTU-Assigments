package Tetris;



public class Tetromino
{

	private char[][] tetro_vector = new char[4][4];

	public Tetromino() {
		this(charimino.I);
	}

	public Tetromino(charimino tetro_type)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				tetro_vector[i][j] = ' ';
			}
		}
    
		switch (tetro_type)
		{ // Fill vector of tetromino according to strong enum class's member
			case I :
				tetro_vector[3][0] = 'I';
				tetro_vector[3][1] = 'I';
				tetro_vector[3][2] = 'I';
				tetro_vector[3][3] = 'I';
				break;
			case O :
				tetro_vector[3][0] = 'O';
				tetro_vector[3][1] = 'O';
				tetro_vector[2][0] = 'O';
				tetro_vector[2][1] = 'O';
				break;
			case T :
				tetro_vector[3][1] = 'T';
				tetro_vector[2][0] = 'T';
				tetro_vector[2][1] = 'T';
				tetro_vector[2][2] = 'T';
				break;
			case J :
				tetro_vector[3][0] = 'J';
				tetro_vector[3][1] = 'J';
				tetro_vector[2][1] = 'J';
				tetro_vector[1][1] = 'J';
				break;
			case L :
				tetro_vector[3][1] = 'L';
				tetro_vector[3][0] = 'L';
				tetro_vector[2][0] = 'L';
				tetro_vector[1][0] = 'L';
				break;
			case S :
				tetro_vector[3][0] = 'S';
				tetro_vector[3][1] = 'S';
				tetro_vector[2][1] = 'S';
				tetro_vector[2][2] = 'S';
				break;
			case Z :
				tetro_vector[3][2] = 'Z';
				tetro_vector[3][1] = 'Z';
				tetro_vector[2][1] = 'Z';
				tetro_vector[2][0] = 'Z';
				break;
		}
	}

	// coppy constructor not named as copy constructor in java but it is
	public Tetromino(Tetromino o)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				tetro_vector[i][j] = o.tetro_vector[i][j];
			}
		}
	}

	// index operator
	public char[] getItem(int i)
	{
		return tetro_vector[i];
	}

	// print tetromino on the screen
	public void print()
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				System.out.print(" ");
				System.out.print(tetro_vector[i][j]);
			}
			System.out.print("\n");
		}
		System.out.print("\n");
		System.out.print("\n");
	}

	//rotate the tetromino
	public void rotate()
	{
		rotate("L");
	}

	public void rotate(String l_or_r)
	{
		if (l_or_r.toLowerCase().equals("left") || l_or_r.toLowerCase().equals("l"))
		{
			rotate_right();
			rotate_right();
			rotate_right();
		}
		else if (l_or_r.toLowerCase().equals("right") || l_or_r.toLowerCase().equals("r"))
		{
			rotate_right();
		}
	}

	private void rotate_right()
	{
		char[][] copy_tet;
		copy_tet = new char[4][4];
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				copy_tet[j][4 - i - 1] = tetro_vector[i][j];
			}
		}
		move_vec(copy_tet);
		boolean delete_line = true;
		for (int i = 0; i < 4 && delete_line; ++i)
		{
			delete_line = true;
			for (int j = 0; j < 4; ++j)
			{
				if (tetro_vector[j][i] != ' ')
				{
					delete_line = false;
				}
			}

			if (delete_line)
			{
				delete_collumn(); // delete empty collumn and lean tetromino against left
				i = -1;
			}

		}
		delete_line = true;
		for (int i = 4 - 1; i > 0 && delete_line; --i)
		{

			delete_line = true;
			for (int j = 0; j < 4; ++j)
			{
				if (tetro_vector[i][j] != ' ')
				{
					delete_line = false;
				}
			}
			if (delete_line)
			{
				delete_row(); // delete empty row and     lean tetromino against bottom
				i = 4;
			}
		}
	}

	//delete empty collumn from tetromino array
	private void delete_collumn()
	{
		char[][] temp;

		temp = new char[4][4];

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 4 - 1; j > 0; --j)
			{
				temp[i][j - 1] = tetro_vector[i][j];
			}
			temp[i][4] = ' ';
		}
		move_vec(temp);
	}

	//delete empty row from tetromino array
	private void delete_row()
	{
		char[] temp;
		temp = tetro_vector[3];

		for (int i = 4 - 1; i > 0 ; --i)
		{
			tetro_vector[i] = tetro_vector[i - 1];
		}
		tetro_vector[0] = temp;
	}

	//  delete tetromino vector for new allocation
	private void move_vec(char[][] o)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				tetro_vector[i][j] = o[i][j];
			}
		}
	}

}

enum charimino {
	I,
	S,
	T,
	Z,
	O,
	L,
	J
}



