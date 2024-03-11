package homework7;

/**
 * The selectionSort class provides a method for sorting a myMap object using the selection sort algorithm.
 */
public class selectionSort extends abstractSort {

    /**
     * Constructs a SelectionSort object with the given map.
     *
     * @param originalMap the original map to be sorted
     */
    public selectionSort(myMap originalMap) {
        super(originalMap);
    }

    /**
     * Constructs a SelectionSort object with default parameters.
     * The originalMap and sortedMap are initialized as empty myMap objects,
     * and the auxiliary list is initialized as an empty ArrayList of Strings.
     */
    public selectionSort() {
        super();
    }


     /**
     * Sorts the auxiliary list using the selection sort algorithm.
     */
    @Override
    protected void sortAux(int left, int right) {
        int n = aux.size();
        for (int i = 0; i < n - 1; i++) {
            int minIndex = i;
            for (int j = i + 1; j < n; j++) {
                if (originalMap.getCount(aux.get(j)) < originalMap.getCount(aux.get(minIndex))) {
                    minIndex = j;
                }
            }
            if (minIndex != i) {
                swapElements(i, minIndex);
            }
        }
    }
    
    /**
     * Swaps two elements in the auxiliary list at the given indices.
     *
     * @param i the index of the first element to be swapped
     * @param j the index of the second element to be swapped
     */
    private void swapElements(int i, int j) {
        String temp = aux.get(i);
        aux.set(i, aux.get(j));
        aux.set(j, temp);
    }
}
