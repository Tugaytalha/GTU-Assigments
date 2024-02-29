package Tetris;




// keeps a location
public class Location implements Comparable<Location>
{ // this class don't need big tree
	public int compareTo(Location otherInstance)
	{
		if (lessThan(otherInstance))
		{
			return -1;
		}
		else if (otherInstance.lessThan(this))
		{
			return 1;
		}

		return 0;
	}

	public int x;
	public int y;
	public boolean moved; //piece moved or not
	// is other in 1 under

	
	public boolean greaterThanOrEqualTo(Location o)
	{
		return ((x == (o.x - 1)) && (y == o.y));
	}

	// is other in 1 above

	
	public boolean lessThanOrEqualTo(Location o)
	{
		return ((x == o.x + 1) && (y == o.y));
	}

	// is other in 1 right

	public boolean greaterThan(Location o)
	{
		return ((x == o.x) && (y == o.y - 1));
	}

	// is other in 1 left

	public boolean lessThan(Location o)
	{
		return ((x == o.x) && (y == o.y + 1));
	}
}