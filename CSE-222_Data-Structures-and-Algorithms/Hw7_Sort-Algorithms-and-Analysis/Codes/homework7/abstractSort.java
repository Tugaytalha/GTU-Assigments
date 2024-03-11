package homework7;

import java.util.ArrayList;

/**
 * The selectionSort class provides a method for sorting a myMap object using the selection sort algorithm.
 */
public abstract class abstractSort implements Cloneable {

    protected myMap originalMap;
    protected myMap sortedMap;
    protected ArrayList<String> aux;

    /**
     * Constructs an AbstractSort object with the given original map.
     *
     * @param originalMap the original map to be sorted
     */
    public abstractSort(myMap originalMap) {
        this.originalMap = originalMap;
        this.aux = originalMap.keyArrayList();
    }

    /**
     * Constructs an AbstractSort object with default parameters.
     * The originalMap and sortedMap are initialized as empty myMap objects,
     * and the auxiliary list is initialized as an empty ArrayList of Strings.
     */
    public abstractSort() {
        this.originalMap = new myMap();
        this.sortedMap = new myMap();
        this.aux = new ArrayList<>();
    }

    /**
     * Sorts the original map and builds a new sorted map using the specific sorting algorithm.
     * Prints the original and sorted maps to the console.
     */
    public void sortMap() {
        System.out.println("The Original (unsorted) map");
        originalMap.show();
        sortAux(0, originalMap.size() - 1);
        buildSortedMap();
        System.out.println("The sorted map");
        sortedMap.show();
    }
    
    /**
     * Sorts the original map and builds a new sorted map using the specific sorting algorithm.
     * And returns running time 
     * 
     * @return executaion time in millisecond
     */
    public double sortMapRT() {
        long execTime = 0;
        
        execTime -= System.nanoTime();
        sortAux(0, originalMap.size() - 1);
        buildSortedMap();
        execTime += System.nanoTime();

        return ((double) execTime / 1000000);
    }

    /**
     * Builds the sorted map based on the sorted keys in the auxiliary list.
     */
    protected void buildSortedMap() {
        sortedMap = new myMap();

        for (int i = 0; i < aux.size(); ++i)
            sortedMap.put(aux.get(i), originalMap.get(aux.get(i)));
    }

    /**
     * Sorts the auxiliary list using a specific sorting algorithm.
     *
     * @param left  the leftmost index of the list to sort
     * @param right the rightmost index of the list to sort
     */
    protected abstract void sortAux(int left, int right);

    /**
     * Creates and returns a copy of this object.
     *
     * @return a clone of this instance.
     * @throws CloneNotSupportedException if the object's class does not support the {@code Cloneable} interface.
     */
    @Override
    protected abstractSort clone() throws CloneNotSupportedException {
        abstractSort clonedSort = (abstractSort) super.clone();
        clonedSort.originalMap = originalMap;
        clonedSort.sortedMap = null;
        clonedSort.aux = new ArrayList<>();
        for (String i : this.aux) {
            clonedSort.aux.add(i);
        }
        return clonedSort;
    }

}
