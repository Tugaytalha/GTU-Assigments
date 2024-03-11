package homework7;

/**
 * This is the main class for running the program to sort maps. 
 * It takes two example input strings, builds a map for each, 
 * sorts each map using mergeSort, and displays the sorted maps.
 */
public class Main {
   
    /**
     * The main method of the program. It creates two example input strings, 
     * builds a map for each, sorts each map using 5 sort algrothms, and displays the sorted maps. 
     * 
     * @param args command-line arguments, not used.
     */
    public static void main(String[] args) {
        String[] exStrs = new String[2];
        exStrs[0] = "Buzzing bees buzz.";
        exStrs[1] = "'Hush, hush!' whispered the rushing wind.";

        myMap[] maps = new myMap[2];
        mergeSort[] msorts = new mergeSort[2];
        selectionSort[] ssorts = new selectionSort[2];
        insertionSort[] isorts = new insertionSort[2];
        bubbleSort[] bsorts = new bubbleSort[2];
        quickSort[] qsorts = new quickSort[2];
    

        for(int i = 0; i < 2; ++i){
            // Creating maps
            maps[i] = new myMap(exStrs[i]);
            maps[i].preProcess();
            maps[i].buildMap();

            // Step a: Merge Sort
            System.out.println("\u001B[36m" + "SORTING WITH MERGE SORT..." + "\u001B[0m");
            msorts[i] = new mergeSort(maps[i]);
            msorts[i].sortMap();
            
            // Step b: Selection Sort
            System.out.println("\u001B[36m" + "SORTING WITH SELECTION SORT..." + "\u001B[0m");
            ssorts[i] = new selectionSort(maps[i]);
            ssorts[i].sortMap();
            
            
            // Step c: Insertion Sort
            System.out.println("\u001B[36m" + "SORTING WITH INSERTION SORT..." + "\u001B[0m");
            isorts[i] = new insertionSort(maps[i]);
            isorts[i].sortMap();
            
            // Step d: Bubble Sort
            System.out.println("\u001B[36m" + "SORTING WITH BUBBLE SORT..." + "\u001B[0m");
            bsorts[i] = new bubbleSort(maps[i]);
            bsorts[i].sortMap();
            
            // Step e: Quick Sort
            System.out.println("\u001B[36m" + "SORTING WITH QUICK SORT..." + "\u001B[0m");
            qsorts[i] = new quickSort(maps[i]);
            qsorts[i].sortMap();
        }
    }

}
