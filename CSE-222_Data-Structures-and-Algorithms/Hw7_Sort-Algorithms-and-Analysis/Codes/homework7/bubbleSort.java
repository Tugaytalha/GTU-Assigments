package homework7;


/**
 * The bubbleSort class provides a method for sorting a myMap object using the bubble sort algorithm.
 */
public class bubbleSort extends abstractSort {

    /**
     * Constructs a bubbleSort object with the given map.
     *
     * @param originalMap the original map to be sorted
     */
    public bubbleSort(myMap originalMap) {
        super(originalMap);
    }

    /**
     * Constructs a bubbleSort object with default parameters.
     * The originalMap and sortedMap are initialized as empty myMap objects,
     * and the auxiliary list is initialized as an empty ArrayList of Strings.
     */
    public bubbleSort() {
        super();
    }

    /**
     * Sorts the auxiliary list using the Bubble Sort algorithm.
     */
    @Override
    protected void sortAux(int left, int right) {
        int n = aux.size();
        boolean swapped = true;
        for (int i = 0; i < n - 1 && swapped; i++) {
            swapped = false;
            for (int j = 0; j < n - i - 1; j++) {
                if (originalMap.getCount(aux.get(j)) > originalMap.getCount(aux.get(j + 1))) {
                    // Swap elements
                    String temp = aux.get(j);
                    aux.set(j, aux.get(j + 1));
                    aux.set(j + 1, temp);
                    swapped = true;
                }
            }
        }
    }
}
