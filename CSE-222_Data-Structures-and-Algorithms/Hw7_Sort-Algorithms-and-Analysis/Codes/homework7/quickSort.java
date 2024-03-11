package homework7;


/**
 * The quickSort class provides a method for sorting a myMap object using the quick sort algorithm.
 */
public class quickSort extends abstractSort {

    /**
     * Constructs a quickSort object with the given map.
     *
     * @param originalMap the original map to be sorted
     */
    public quickSort(myMap originalMap) {
        super(originalMap);
    }

    /**
     * Constructs a quickSort object with default parameters.
     * The originalMap and sortedMap are initialized as empty myMap objects,
     * and the auxiliary list is initialized as an empty ArrayList of Strings.
     */
    public quickSort() {
        super();
    }

    /**
     * Sorts the auxiliary list using the quick sort algorithm.
     *
     * @param left  the leftmost index of the list to sort
     * @param right the rightmost index of the list to sort
     */
    @Override
    protected void sortAux(int left, int right) {
        if (left < right) {
            int partitionIndex = partition(left, right);
            sortAux(left, partitionIndex - 1);
            sortAux(partitionIndex + 1, right);
        }
    }

    /**
     * Partitions the auxiliary list and returns the partition index.
     *
     * @param left  the leftmost index of the list to partition
     * @param right the rightmost index of the list to partition
     * @return the partition index
     */
    private int partition(int left, int right) {
        String pivot = aux.get(right);
        int i = left - 1;

        for (int j = left; j < right; j++) {
            if (originalMap.getCount(aux.get(j)) <= originalMap.getCount(pivot)) {
                i++;
                swap(i, j);
            }
        }

        swap(i + 1, right);
        return i + 1;
    }

    /**
     * Swaps two elements in the auxiliary list.
     *
     * @param i the index of the first element
     * @param j the index of the second element
     */
    private void swap(int i, int j) {
        String temp = aux.get(i);
        aux.set(i, aux.get(j));
        aux.set(j, temp);
    }
}
