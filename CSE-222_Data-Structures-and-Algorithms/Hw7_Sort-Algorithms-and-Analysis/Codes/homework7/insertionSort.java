package homework7;


/**
 * The insertionSort class provides a method for sorting a myMap object using the insertion sort algorithm.
 */
public class insertionSort extends abstractSort {

    /**
     * Constructs an insertionSort object with the given map.
     *
     * @param originalMap the original map to be sorted
     */
    public insertionSort(myMap originalMap) {
        super(originalMap);
    }

    /**
     * Constructs an insertionSort object with default parameters.
     * The originalMap and sortedMap are initialized as empty myMap objects,
     * and the auxiliary list is initialized as an empty ArrayList of Strings.
     */
    public insertionSort() {
        super();
    }

    /**
     * Sorts the auxiliary list using the insertion sort algorithm.
     *
     * @param left  the leftmost index of the list to sort
     * @param right the rightmost index of the list to sort
     */
    @Override
    protected void sortAux(int left, int right) {
        int n = aux.size();

        for (int i = 1; i < n; i++) {
            String key = aux.get(i);
            int j = i - 1;

            while (j >= 0 && originalMap.getCount(aux.get(j)) > originalMap.getCount(key)) {
                aux.set(j + 1, aux.get(j));
                j--;
            }

            aux.set(j + 1, key);
        }
    }
}
