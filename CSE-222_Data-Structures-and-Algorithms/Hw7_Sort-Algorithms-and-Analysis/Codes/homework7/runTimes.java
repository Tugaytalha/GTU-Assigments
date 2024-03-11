package homework7;

/**
 * This is the main class for running the program to sort maps. 
 * It takes three example input strings, builds a map for each, 
 * sorts each map using  5 diffrent sorting algorihtms, and 
 * printing average running times. 
 */
public class runTimes {
   
    /**
     * The main method of the program. It creates three example input strings, 
     * builds a map for each, sorts each map using 5 diffrent sorting 
     * algorihtms, and printing average running times. 
     * 
     * @param args command-line arguments, not used.
     */
    public static void main(String[] args) {
        String[] exStrs = new String[3];
        // String for Best case
        exStrs[0] = "DCBA SSSS TTTT TTTT TTTT SSSS SS ABCD ABCA ABAB AAAA";
        // String for Average case
        exStrs[1] = "DSBC TAAA AATA DSSS SS SSSS TTTT ABCT ABCT ABAB TTTT";
        // String for Worst case
        exStrs[2] = "TTTT TTTT TTTT SSSS SSSS SS ABCD ABCD ABCA ABAB AAAA";

        myMap[] maps = new myMap[3];
        mergeSort[] msorts = new mergeSort[3];
        selectionSort[] ssorts = new selectionSort[3];
        insertionSort[] isorts = new insertionSort[3];
        bubbleSort[] bsorts = new bubbleSort[3];
        quickSort[] qsorts = new quickSort[3];

        // Creating maps and sort objects
        for (int i = 0; i < 3; i++) {
            maps[i] = new myMap(exStrs[i]);
            maps[i].preProcess();
            maps[i].buildMap();
            ssorts[i] = new selectionSort(maps[i]);
            isorts[i] = new insertionSort(maps[i]);
            bsorts[i] = new bubbleSort(maps[i]);
            
        }
        /*
         * For quick sort the worst case is elements are normal or reverse sorted 
         * in other word there are 2 worst case instead of best-average-worst case 
         * 
         * Similarly for merge sort's cases are diffrent then other sorts worst case 
         * needs maximum comparisons like for 0,1,2,3,4,5 -> 0,4,2,1,5,3 
         */
        
        msorts[0] = new mergeSort(maps[0]);
        msorts[1] = new mergeSort(maps[2]);
        msorts[2] = new mergeSort(maps[1]);
        qsorts[0] = new quickSort(maps[1]);
        qsorts[1] = new quickSort(maps[2]);
        qsorts[2] = new quickSort(maps[0]);
        

        try {
            // Step a: Merge Sort
            System.out.println("\u001B[36m" + "SORTING WITH MERGE SORT..." + "\u001B[0m");
            avrgSortTimes(maps, msorts);
            
            // Step b: Selection Sort
            System.out.println("\u001B[36m" + "SORTING WITH SELECTION SORT..." + "\u001B[0m");
            avrgSortTimes(maps, ssorts);
            
            // Step c: Insertion Sort
            System.out.println("\u001B[36m" + "SORTING WITH INSERTION SORT..." + "\u001B[0m");
            avrgSortTimes(maps, isorts);
            
            // Step d: Bubble Sort
            System.out.println("\u001B[36m" + "SORTING WITH BUBBLE SORT..." + "\u001B[0m");
            avrgSortTimes(maps, bsorts);
            
            // Step e: Quick Sort
            System.out.println("\u001B[36m" + "SORTING WITH QUICK SORT..." + "\u001B[0m");
            avrgSortTimes(maps, qsorts);
            
        } catch (Exception e) {
            System.out.println(e.getMessage() + "should be handled");
        }
        
    }
    
    /**
     * Calculates and prints the average execution times for different sorting scenarios.
     * 
     * @param maps  An array of myMap objects to be sorted.
     * @param sorts An array of abstractSort objects representing the sorting algorithms.
     */
    private static void avrgSortTimes(myMap[] maps, abstractSort[] sorts) throws CloneNotSupportedException {
        double execTime = 0;
        abstractSort[] orginals = new abstractSort[3];
        orginals[0] = sorts[0].clone(); 
        orginals[1] = sorts[1].clone(); 
        orginals[2] = sorts[2].clone(); 
        
        for(int i = 0; i < 100; i++) {
            execTime += sorts[2].sortMapRT();
            sorts[2] = orginals[2].clone();
        }  
        System.out.printf("    Worst case:   %.6f ms\n", (execTime / 100));      
        execTime = 0;

        for(int i = 0; i < 100; i++) {
            execTime += sorts[1].sortMapRT();
            sorts[1] = orginals[1].clone();
        }  
        System.out.printf("    Avarage case: %.6f ms\n", (execTime / 100));      
        execTime = 0;
        
        for(int i = 0; i < 100; i++) {
            execTime += sorts[0].sortMapRT();
            sorts[0] = orginals[0].clone();
        }  
        System.out.printf("    Best case:    %.6f ms\n", (execTime / 100));      
        execTime = 0;
    
    }

}
