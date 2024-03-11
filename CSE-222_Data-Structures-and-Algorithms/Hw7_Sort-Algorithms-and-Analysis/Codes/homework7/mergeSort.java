package homework7;


/**
 * The mergeSort class provides a method for sorting a myMap object using the merge sort algorithm.
 */
public class mergeSort extends abstractSort {

    /**
     * Constructs a mergeSort object with the given map.
     *
     * @param originalMap the original map to be sorted
     */
    public mergeSort(myMap originalMap) {
        super(originalMap);
    }

    /**
     * Constructs a mergeSort object with default parameters.
     * The originalMap and sortedMap are initialized as empty myMap objects,
     * and the auxiliary list is initialized as an empty ArrayList of Strings.
     */
    public mergeSort() {
        super();
    }

    /**
    /**
     * Sorts the auxiliary list using the merge sort algorithm.
     *
     * @param left  the leftmost index of the list to sort
     * @param right the rightmost index of the list to sort
     */
    @Override
    protected void sortAux(int left, int right) {
        if (left >= right)
            return;

        int mid = (left + right) / 2;
        sortAux(left, mid);
        sortAux(mid + 1, right);
        merge(left, mid, right);
    }


    /**
     * Merges two sorted sublists of the auxiliary list into one sorted sublist.
     *
     * @param left the leftmost index of the left sublist
     * @param mid the rightmost index of the left sublist
     * @param right the rightmost index of the right sublist
     */
    private void merge(int left, int mid, int right) {
        int i = left;
        int j = mid + 1;
        int k = left;
    
        sortedMap = new myMap();

        while (i <= mid && j <= right) {
            if (originalMap.getCount(aux.get(i)) <= originalMap.getCount(aux.get(j))) {
                sortedMap.put(aux.get(i), originalMap.get(aux.get(i)));
                i++;
            } else {
                sortedMap.put(aux.get(j), originalMap.get(aux.get(j)));
                j++;
            }
            k++;
        }
    
        while (i <= mid) {
            sortedMap.put(aux.get(i), originalMap.get(aux.get(i)));
            i++;
            k++;
        }
    
        while (j <= right) {
            sortedMap.put(aux.get(j), originalMap.get(aux.get(j)));
            j++;
            k++;
        }
    
        for (k = 0; k <= right - left; k++) {
            aux.set(k + left, sortedMap.keyArrayList().get(k));
        }
    }
    


}

