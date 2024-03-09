package homework6;

import java.util.ArrayList;


/**
 * The mergeSort class provides a method for sorting a myMap object using the merge sort algorithm.
 */
public class mergeSort {

    myMap originalMap;
    myMap sortedMap;
    ArrayList<String> aux;
   
    /** 
    /**
     * Constructs a mergeSort object with the given parameters.
     *
     * @param originalMap the original map to be sorted
     * @param sortedMap the sorted map
     * @param aux the auxiliary list used for sorting
     *
    public mergeSort(myMap originalMap, List<String> aux) {
        this.originalMap = originalMap;
        this.sortedMap = originalMap.clone();
        this.aux = aux;
    }
    */
    

    /**
     * Constructs a mergeSort object with the given map.
     *
     * @param originalMap the original map to be sorted
     */
    public mergeSort(myMap originalMap) {
        this.originalMap = originalMap;
        this.aux = originalMap.keyArrayList();
    }

    /**
     * Constructs a mergeSort object with default parameters.
     * The originalMap and sortedMap are initialized as empty myMap objects,
     * and the auxiliary list is initialized as an empty ArrayList of Strings.
     */
    public mergeSort() {
        this.originalMap = new myMap();
        this.sortedMap = new myMap();
        this.aux = new ArrayList<String>();
    }

    /**
     * Sorts the original map and builds a new sorted map using 
     * mergeSort alghoritm based on the letters accurance count.
     * Prints the original and sorted maps to the console.
     */
    public void sortMap() {

        System.out.println("The ORginal (unsorted) map");
        originalMap.show();
        sortAux(0, originalMap.size() - 1);
        buildSortedMap();
        System.out.println("The sorted map");
        sortedMap.show();
    }

    /**
     * Builds the sorted map based on the sorted keys in the auxiliary list.
     */
    private void buildSortedMap() {
        sortedMap = new myMap();

        for(int i = 0; i < aux.size(); ++i)
            sortedMap.put(aux.get(i), originalMap.get(aux.get(i)));
    }

    /**
     * Sorts the auxiliary list using the merge sort algorithm.
     *
     * @param left the leftmost index of the list to sort
     * @param right the rightmost index of the list to sort
     */
    private void sortAux(int left, int right) {
        if(left >= right)
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

